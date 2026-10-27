// Copyright (c) 2026 F. Lotfi All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "IR/IrOptimizer.h"
#include <algorithm>
#include <cctype>
#include <cstdint>
#include <cstdlib>
#include <limits>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace {

bool IsTempRegister(const std::string &name) {
  if (name.size() < 2 || name[0] != '%') {
    return false;
  }
  for (size_t i = 1; i < name.size(); ++i) {
    if (!std::isdigit(static_cast<unsigned char>(name[i]))) {
      return false;
    }
  }
  return true;
}

bool IsPureInstruction(IrOpcode op) {
  return op == IrOpcode::Const || IsUnaryOp(op) || IsBinaryOp(op);
}

bool TryParseConstValue(const std::string &text, Value &out) {
  if (text == kIrTrueLiteral) {
    out = true;
    return true;
  }
  if (text == kIrFalseLiteral) {
    out = false;
    return true;
  }
  if (text.size() >= 2 && text.front() == '"' && text.back() == '"') {
    out = UnescapeIrString(text);
    return true;
  }
  if (IsIrFloatLiteral(text)) {
    char *endPtr = nullptr;
    errno = 0;
    double parsed = std::strtod(text.c_str(), &endPtr);
    if (errno != 0 || endPtr != text.c_str() + text.size()) {
      return false;
    }
    out = parsed;
    return true;
  }
  if (text.empty()) {
    return false;
  }
  char *endPtr = nullptr;
  errno = 0;
  long parsed = std::strtol(text.c_str(), &endPtr, 10);
  if (errno != 0 || endPtr != text.c_str() + text.size()) {
    return false;
  }
  if (parsed < std::numeric_limits<int32_t>::min() ||
      parsed > std::numeric_limits<int32_t>::max()) {
    return false;
  }
  out = static_cast<int32_t>(parsed);
  return true;
}

std::string FormatConstValue(const Value &value) {
  if (value.isBool()) {
    return value.asBool() ? kIrTrueLiteral : kIrFalseLiteral;
  }
  if (value.isFloat()) {
    return FormatIrFloat(value.asFloat());
  }
  if (value.isString()) {
    return EscapeIrString(value.asString());
  }
  return std::to_string(value.asInt());
}

bool TryEvaluateUnary(IrOpcode op, const Value &operand, Value &result) {
  switch (op) {
  case IrOpcode::Copy:
    if (!operand.isInt() && !operand.isFloat() && !operand.isBool() &&
        !operand.isString()) {
      return false;
    }
    result = operand;
    return true;
  case IrOpcode::Neg:
    if (operand.isFloat()) {
      result = -operand.asFloat();
      return true;
    }
    if (!operand.isInt()) {
      return false;
    }
    result = static_cast<int32_t>(-static_cast<int64_t>(operand.asInt()));
    return true;
  case IrOpcode::Not:
    if (!operand.isBool()) {
      return false;
    }
    result = !operand.asBool();
    return true;
  case IrOpcode::BitwiseNot:
    if (!operand.isInt()) {
      return false;
    }
    result = ~operand.asInt();
    return true;
  default:
    return false;
  }
}

bool TryEvaluateBinary(IrOpcode op, const Value &left, const Value &right,
                       Value &result) {
  switch (op) {
  case IrOpcode::Add:
    if (left.isString() && right.isString()) {
      result = left.asString() + right.asString();
      return true;
    }
    if (left.isFloat() && right.isFloat()) {
      result = left.asFloat() + right.asFloat();
      return true;
    }
    if (!left.isInt() || !right.isInt()) {
      return false;
    }
    result = static_cast<int32_t>(static_cast<int64_t>(left.asInt()) +
                                  static_cast<int64_t>(right.asInt()));
    return true;
  case IrOpcode::Sub:
    if (left.isFloat() && right.isFloat()) {
      result = left.asFloat() - right.asFloat();
      return true;
    }
    if (!left.isInt() || !right.isInt()) {
      return false;
    }
    result = static_cast<int32_t>(static_cast<int64_t>(left.asInt()) -
                                  static_cast<int64_t>(right.asInt()));
    return true;
  case IrOpcode::Mul:
    if (left.isFloat() && right.isFloat()) {
      result = left.asFloat() * right.asFloat();
      return true;
    }
    if (!left.isInt() || !right.isInt()) {
      return false;
    }
    result = static_cast<int32_t>(static_cast<int64_t>(left.asInt()) *
                                  static_cast<int64_t>(right.asInt()));
    return true;
  case IrOpcode::Div:
    if (left.isFloat() && right.isFloat()) {
      if (right.asFloat() == 0.0) {
        return false;
      }
      result = left.asFloat() / right.asFloat();
      return true;
    }
    if (!left.isInt() || !right.isInt() || right.asInt() == 0) {
      return false;
    }
    if (left.asInt() == std::numeric_limits<int32_t>::min() &&
        right.asInt() == -1) {
      return false;
    }
    result = left.asInt() / right.asInt();
    return true;
  case IrOpcode::Mod:
    if (!left.isInt() || !right.isInt() || right.asInt() == 0) {
      return false;
    }
    if (left.asInt() == std::numeric_limits<int32_t>::min() &&
        right.asInt() == -1) {
      result = 0;
      return true;
    }
    result = left.asInt() % right.asInt();
    return true;
  case IrOpcode::BitwiseAnd:
    if (!left.isInt() || !right.isInt()) {
      return false;
    }
    result = left.asInt() & right.asInt();
    return true;
  case IrOpcode::BitwiseOr:
    if (!left.isInt() || !right.isInt()) {
      return false;
    }
    result = left.asInt() | right.asInt();
    return true;
  case IrOpcode::Xor:
    if (!left.isInt() || !right.isInt()) {
      return false;
    }
    result = left.asInt() ^ right.asInt();
    return true;
  case IrOpcode::Eq:
    if ((left.isInt() && right.isInt()) || (left.isBool() && right.isBool())) {
      result = left.isInt() ? (left.asInt() == right.asInt())
                            : (left.asBool() == right.asBool());
      return true;
    }
    if (left.isFloat() && right.isFloat()) {
      result = (left.asFloat() == right.asFloat());
      return true;
    }
    if (left.isString() && right.isString()) {
      result = (left.asString() == right.asString());
      return true;
    }
    return false;
  case IrOpcode::Ne:
    if ((left.isInt() && right.isInt()) || (left.isBool() && right.isBool())) {
      result = left.isInt() ? (left.asInt() != right.asInt())
                            : (left.asBool() != right.asBool());
      return true;
    }
    if (left.isFloat() && right.isFloat()) {
      result = (left.asFloat() != right.asFloat());
      return true;
    }
    if (left.isString() && right.isString()) {
      result = (left.asString() != right.asString());
      return true;
    }
    return false;
  case IrOpcode::And:
    if (!left.isBool() || !right.isBool()) {
      return false;
    }
    result = left.asBool() && right.asBool();
    return true;
  case IrOpcode::Or:
    if (!left.isBool() || !right.isBool()) {
      return false;
    }
    result = left.asBool() || right.asBool();
    return true;
  case IrOpcode::Gt:
    if (left.isFloat() && right.isFloat()) {
      result = left.asFloat() > right.asFloat();
      return true;
    }
    if (!left.isInt() || !right.isInt()) {
      return false;
    }
    result = left.asInt() > right.asInt();
    return true;
  case IrOpcode::Lt:
    if (left.isFloat() && right.isFloat()) {
      result = left.asFloat() < right.asFloat();
      return true;
    }
    if (!left.isInt() || !right.isInt()) {
      return false;
    }
    result = left.asInt() < right.asInt();
    return true;
  case IrOpcode::Le:
    if (left.isFloat() && right.isFloat()) {
      result = left.asFloat() <= right.asFloat();
      return true;
    }
    if (!left.isInt() || !right.isInt()) {
      return false;
    }
    result = left.asInt() <= right.asInt();
    return true;
  case IrOpcode::Ge:
    if (left.isFloat() && right.isFloat()) {
      result = left.asFloat() >= right.asFloat();
      return true;
    }
    if (!left.isInt() || !right.isInt()) {
      return false;
    }
    result = left.asInt() >= right.asInt();
    return true;
  default:
    return false;
  }
}

bool FoldConstants(IrFunction &function) {
  std::unordered_map<std::string, size_t> defCounts;
  std::unordered_set<std::string> storedRegs;
  for (const auto &inst : function.instructions) {
    if (!inst.result.empty()) {
      defCounts[inst.result]++;
      if (IsStoreOp(inst.op)) {
        storedRegs.insert(inst.result);
      }
    }
  }

  auto isSsaTemp = [&](const std::string &reg) {
    auto it = defCounts.find(reg);
    return IsTempRegister(reg) && it != defCounts.end() && it->second == 1 &&
           storedRegs.find(reg) == storedRegs.end();
  };

  std::unordered_map<std::string, Value> constants;
  auto clearNonTempConstants = [&]() {
    for (auto it = constants.begin(); it != constants.end();) {
      if (!isSsaTemp(it->first)) {
        it = constants.erase(it);
      } else {
        ++it;
      }
    }
  };

  auto resolveConst = [&](const std::string &operand, Value &val) -> bool {
    auto it = constants.find(operand);
    if (it != constants.end()) {
      val = it->second;
      return true;
    }
    return TryParseConstValue(operand, val);
  };

  bool changed = false;
  for (auto &inst : function.instructions) {
    if (inst.op == IrOpcode::Label) {
      clearNonTempConstants();
      continue;
    }

    if (inst.op == IrOpcode::Const) {
      Value value;
      if (inst.operands.size() == 1 && !inst.result.empty() &&
          TryParseConstValue(inst.operands[0], value)) {
        if (isSsaTemp(inst.result) ||
            storedRegs.find(inst.result) == storedRegs.end()) {
          constants[inst.result] = value;
        }
      }
      continue;
    }

    if (IsUnaryOp(inst.op) && inst.operands.size() == 1) {
      Value operandVal;
      Value folded;
      if (resolveConst(inst.operands[0], operandVal) &&
          TryEvaluateUnary(inst.op, operandVal, folded)) {
        inst.op = IrOpcode::Const;
        inst.operands = {FormatConstValue(folded)};
        changed = true;
        if (!inst.result.empty() &&
            (isSsaTemp(inst.result) ||
             storedRegs.find(inst.result) == storedRegs.end())) {
          constants[inst.result] = folded;
        }
        continue;
      }
    }

    if (IsBinaryOp(inst.op) && inst.operands.size() == 2) {
      Value leftVal;
      Value rightVal;
      Value folded;
      if (resolveConst(inst.operands[0], leftVal) &&
          resolveConst(inst.operands[1], rightVal) &&
          TryEvaluateBinary(inst.op, leftVal, rightVal, folded)) {
        inst.op = IrOpcode::Const;
        inst.operands = {FormatConstValue(folded)};
        changed = true;
        if (!inst.result.empty() &&
            (isSsaTemp(inst.result) ||
             storedRegs.find(inst.result) == storedRegs.end())) {
          constants[inst.result] = folded;
        }
        continue;
      }
    }

    if (!inst.result.empty() && !isSsaTemp(inst.result)) {
      constants.erase(inst.result);
    }
  }

  return changed;
}

bool EliminateDeadTemporaries(IrFunction &function) {
  bool anyRemoved = false;
  bool removedInPass = true;
  while (removedInPass) {
    removedInPass = false;
    std::unordered_map<std::string, size_t> defCounts;
    std::unordered_set<std::string> storedRegs;
    std::unordered_map<std::string, size_t> useCounts;

    for (const auto &inst : function.instructions) {
      if (!inst.result.empty()) {
        defCounts[inst.result]++;
        if (IsStoreOp(inst.op)) {
          storedRegs.insert(inst.result);
        }
      }
      for (const auto &operand : inst.operands) {
        useCounts[operand]++;
      }
      if (IsCompoundStoreOp(inst.op) && !inst.result.empty()) {
        useCounts[inst.result]++;
      }
    }

    if (!function.returnValue.empty()) {
      useCounts[function.returnValue]++;
    }

    auto newEnd = std::remove_if(
        function.instructions.begin(), function.instructions.end(),
        [&](const IrInstruction &inst) {
          if (!IsPureInstruction(inst.op) || inst.result.empty() ||
              !IsTempRegister(inst.result)) {
            return false;
          }
          if (defCounts[inst.result] != 1 ||
              storedRegs.find(inst.result) != storedRegs.end()) {
            return false;
          }
          return useCounts[inst.result] == 0;
        });

    if (newEnd != function.instructions.end()) {
      function.instructions.erase(newEnd, function.instructions.end());
      removedInPass = true;
      anyRemoved = true;
    }
  }
  return anyRemoved;
}

void OptimizeFunction(IrFunction &function) {
  bool changed = true;
  while (changed) {
    bool folded = FoldConstants(function);
    bool eliminated = EliminateDeadTemporaries(function);
    changed = folded || eliminated;
  }
}

} // namespace

IrModule IrOptimizer::Optimize(const IrModule &module) {
  IrModule copy = module;
  OptimizeInPlace(copy);
  return copy;
}

void IrOptimizer::OptimizeInPlace(IrModule &module) {
  for (auto &function : module.functions) {
    OptimizeFunction(function);
  }
}
