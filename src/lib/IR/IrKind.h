/*
 * Copyright (c) 2026 F. Lotfi All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>

enum class IrOpcode : uint8_t {
  Unknown,
  UnsupportedStatement,
  Const,
  Copy,
  Neg,
  Not,
  BitwiseNot,
  Add,
  Sub,
  Mul,
  Div,
  Mod,
  Eq,
  Ne,
  And,
  Or,
  BitwiseAnd,
  BitwiseOr,
  Xor,
  Gt,
  Lt,
  Le,
  Ge,
  Store,
  StoreAdd,
  StoreSub,
  StoreMul,
  StoreDiv,
  StoreMod,
  StoreBitwiseAnd,
  StoreBitwiseOr,
  StoreXor,
  Call,
  Print,
  Label,
  Br,
  BrCond,
  Ret,
};

enum class IrLabelKind : uint8_t {
  Entry,
  Then,
  Else,
  End,
  LogicRhs,
  LogicEnd,
  WhileCond,
  WhileBody,
  WhileEnd,
  DoBody,
  DoCond,
  DoEnd,
  ForCond,
  ForBody,
  ForInc,
  ForEnd,
};

struct IrOpcodeHash {
  size_t operator()(IrOpcode op) const { return static_cast<size_t>(op); }
};

struct IrLabelKindHash {
  size_t operator()(IrLabelKind kind) const {
    return static_cast<size_t>(kind);
  }
};

static constexpr const char *kIrMainFunctionName = "__main";
static constexpr const char *kIrVoidValue = "void";
static constexpr const char *kIrTrueLiteral = "true";
static constexpr const char *kIrFalseLiteral = "false";

static const std::unordered_map<IrOpcode, std::string, IrOpcodeHash>
    IrOpcodeToStrMap = {
        {IrOpcode::Unknown, "unknown"},
        {IrOpcode::UnsupportedStatement, "unsupported_statement"},
        {IrOpcode::Const, "const"},
        {IrOpcode::Copy, "copy"},
        {IrOpcode::Neg, "neg"},
        {IrOpcode::Not, "not"},
        {IrOpcode::BitwiseNot, "bnot"},
        {IrOpcode::Add, "add"},
        {IrOpcode::Sub, "sub"},
        {IrOpcode::Mul, "mul"},
        {IrOpcode::Div, "div"},
        {IrOpcode::Mod, "mod"},
        {IrOpcode::Eq, "eq"},
        {IrOpcode::Ne, "ne"},
        {IrOpcode::And, "and"},
        {IrOpcode::Or, "or"},
        {IrOpcode::BitwiseAnd, "band"},
        {IrOpcode::BitwiseOr, "bor"},
        {IrOpcode::Xor, "xor"},
        {IrOpcode::Gt, "gt"},
        {IrOpcode::Lt, "lt"},
        {IrOpcode::Le, "le"},
        {IrOpcode::Ge, "ge"},
        {IrOpcode::Store, "store"},
        {IrOpcode::StoreAdd, "store_add"},
        {IrOpcode::StoreSub, "store_sub"},
        {IrOpcode::StoreMul, "store_mul"},
        {IrOpcode::StoreDiv, "store_div"},
        {IrOpcode::StoreMod, "store_mod"},
        {IrOpcode::StoreBitwiseAnd, "store_band"},
        {IrOpcode::StoreBitwiseOr, "store_bor"},
        {IrOpcode::StoreXor, "store_xor"},
        {IrOpcode::Call, "call"},
        {IrOpcode::Print, "print"},
        {IrOpcode::Label, "label"},
        {IrOpcode::Br, "br"},
        {IrOpcode::BrCond, "br_cond"},
        {IrOpcode::Ret, "ret"},
};

static const std::unordered_map<std::string, IrOpcode> StrToIrOpcodeMap = {
    {"unknown", IrOpcode::Unknown},
    {"unsupported_statement", IrOpcode::UnsupportedStatement},
    {"const", IrOpcode::Const},
    {"copy", IrOpcode::Copy},
    {"neg", IrOpcode::Neg},
    {"not", IrOpcode::Not},
    {"bnot", IrOpcode::BitwiseNot},
    {"add", IrOpcode::Add},
    {"sub", IrOpcode::Sub},
    {"mul", IrOpcode::Mul},
    {"div", IrOpcode::Div},
    {"mod", IrOpcode::Mod},
    {"eq", IrOpcode::Eq},
    {"ne", IrOpcode::Ne},
    {"and", IrOpcode::And},
    {"or", IrOpcode::Or},
    {"band", IrOpcode::BitwiseAnd},
    {"bor", IrOpcode::BitwiseOr},
    {"xor", IrOpcode::Xor},
    {"gt", IrOpcode::Gt},
    {"lt", IrOpcode::Lt},
    {"le", IrOpcode::Le},
    {"ge", IrOpcode::Ge},
    {"store", IrOpcode::Store},
    {"store_add", IrOpcode::StoreAdd},
    {"store_sub", IrOpcode::StoreSub},
    {"store_mul", IrOpcode::StoreMul},
    {"store_div", IrOpcode::StoreDiv},
    {"store_mod", IrOpcode::StoreMod},
    {"store_band", IrOpcode::StoreBitwiseAnd},
    {"store_bor", IrOpcode::StoreBitwiseOr},
    {"store_xor", IrOpcode::StoreXor},
    {"call", IrOpcode::Call},
    {"print", IrOpcode::Print},
    {"label", IrOpcode::Label},
    {"br", IrOpcode::Br},
    {"br_cond", IrOpcode::BrCond},
    {"ret", IrOpcode::Ret},
};

static const std::unordered_map<IrLabelKind, std::string, IrLabelKindHash>
    IrLabelKindToPrefixMap = {
        {IrLabelKind::Entry, "entry"},
        {IrLabelKind::Then, "then"},
        {IrLabelKind::Else, "else"},
        {IrLabelKind::End, "end"},
        {IrLabelKind::LogicRhs, "logic_rhs"},
        {IrLabelKind::LogicEnd, "logic_end"},
        {IrLabelKind::WhileCond, "while_cond"},
        {IrLabelKind::WhileBody, "while_body"},
        {IrLabelKind::WhileEnd, "while_end"},
        {IrLabelKind::DoBody, "do_body"},
        {IrLabelKind::DoCond, "do_cond"},
        {IrLabelKind::DoEnd, "do_end"},
        {IrLabelKind::ForCond, "for_cond"},
        {IrLabelKind::ForBody, "for_body"},
        {IrLabelKind::ForInc, "for_inc"},
        {IrLabelKind::ForEnd, "for_end"},
};

inline const std::string &IrOpcodeToString(IrOpcode op) {
  return IrOpcodeToStrMap.at(op);
}

inline IrOpcode IrOpcodeFromString(const std::string &text) {
  auto it = StrToIrOpcodeMap.find(text);
  return it == StrToIrOpcodeMap.end() ? IrOpcode::Unknown : it->second;
}

inline const std::string &IrLabelPrefix(IrLabelKind kind) {
  return IrLabelKindToPrefixMap.at(kind);
}

inline std::string MakeIrLabel(IrLabelKind kind, int32_t id) {
  return IrLabelPrefix(kind) + std::to_string(id);
}

inline bool IsUnaryOp(IrOpcode op) {
  switch (op) {
  case IrOpcode::Copy:
  case IrOpcode::Neg:
  case IrOpcode::Not:
  case IrOpcode::BitwiseNot:
    return true;
  default:
    return false;
  }
}

inline bool IsComparisonOp(IrOpcode op) {
  switch (op) {
  case IrOpcode::Eq:
  case IrOpcode::Ne:
  case IrOpcode::Lt:
  case IrOpcode::Le:
  case IrOpcode::Gt:
  case IrOpcode::Ge:
    return true;
  default:
    return false;
  }
}

inline bool IsBinaryOp(IrOpcode op) {
  switch (op) {
  case IrOpcode::Add:
  case IrOpcode::Sub:
  case IrOpcode::Mul:
  case IrOpcode::Div:
  case IrOpcode::Mod:
  case IrOpcode::And:
  case IrOpcode::Or:
  case IrOpcode::BitwiseAnd:
  case IrOpcode::BitwiseOr:
  case IrOpcode::Xor:
    return true;
  default:
    return IsComparisonOp(op);
  }
}

inline bool IsCompoundStoreOp(IrOpcode op) {
  switch (op) {
  case IrOpcode::StoreAdd:
  case IrOpcode::StoreSub:
  case IrOpcode::StoreMul:
  case IrOpcode::StoreDiv:
  case IrOpcode::StoreMod:
  case IrOpcode::StoreBitwiseAnd:
  case IrOpcode::StoreBitwiseOr:
  case IrOpcode::StoreXor:
    return true;
  default:
    return false;
  }
}

inline bool IsStoreOp(IrOpcode op) {
  return op == IrOpcode::Store || IsCompoundStoreOp(op);
}

inline bool IsTerminatorOp(IrOpcode op) {
  return op == IrOpcode::Ret || op == IrOpcode::Br || op == IrOpcode::BrCond;
}

inline IrOpcode CompoundStoreToBinaryOp(IrOpcode op) {
  switch (op) {
  case IrOpcode::StoreAdd:
    return IrOpcode::Add;
  case IrOpcode::StoreSub:
    return IrOpcode::Sub;
  case IrOpcode::StoreMul:
    return IrOpcode::Mul;
  case IrOpcode::StoreDiv:
    return IrOpcode::Div;
  case IrOpcode::StoreMod:
    return IrOpcode::Mod;
  case IrOpcode::StoreBitwiseAnd:
    return IrOpcode::BitwiseAnd;
  case IrOpcode::StoreBitwiseOr:
    return IrOpcode::BitwiseOr;
  case IrOpcode::StoreXor:
    return IrOpcode::Xor;
  default:
    return IrOpcode::Unknown;
  }
}

inline std::string EscapeIrString(const std::string &str) {
  std::string result = "\"";
  for (char c : str) {
    switch (c) {
    case '\\':
      result += "\\\\";
      break;
    case '\"':
      result += "\\\"";
      break;
    case '\n':
      result += "\\n";
      break;
    case '\t':
      result += "\\t";
      break;
    default:
      result += c;
      break;
    }
  }
  result += "\"";
  return result;
}

inline std::string UnescapeIrString(const std::string &quoted) {
  if (quoted.size() < 2 || quoted.front() != '"' || quoted.back() != '"') {
    return quoted;
  }
  std::string result;
  result.reserve(quoted.size() - 2);
  for (size_t i = 1; i + 1 < quoted.size(); ++i) {
    if (quoted[i] == '\\' && i + 2 < quoted.size()) {
      ++i;
      switch (quoted[i]) {
      case 'n':
        result.push_back('\n');
        break;
      case 't':
        result.push_back('\t');
        break;
      case '"':
        result.push_back('"');
        break;
      case '\\':
        result.push_back('\\');
        break;
      default:
        result.push_back('\\');
        result.push_back(quoted[i]);
        break;
      }
    } else {
      result.push_back(quoted[i]);
    }
  }
  return result;
}
