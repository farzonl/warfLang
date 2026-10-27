// Copyright (c) 2026 F. Lotfi All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "Ir.h"
#include "Binding/BoundAssignmentExpressionNode.h"
#include "Binding/BoundBinaryExpressionNode.h"
#include "Binding/BoundBlockStatementNode.h"
#include "Binding/BoundBreakStatementNode.h"
#include "Binding/BoundCallExpressionNode.h"
#include "Binding/BoundContinueStatementNode.h"
#include "Binding/BoundDoWhileStatementNode.h"
#include "Binding/BoundExpressionStatementNode.h"
#include "Binding/BoundForStatementNode.h"
#include "Binding/BoundFunctionDeclarationNode.h"
#include "Binding/BoundIdentifierExpressionNode.h"
#include "Binding/BoundIfStatementNode.h"
#include "Binding/BoundLiteralExpressionNode.h"
#include "Binding/BoundReturnStatementNode.h"
#include "Binding/BoundUnaryExpressionNode.h"
#include "Binding/BoundVariableDeclarationNode.h"
#include "Binding/BoundWhileStatementNode.h"
#include "Symbol/VariableSymbol.h"
#include <algorithm>
#include <cctype>
#include <iostream>
#include <sstream>
#include <stdexcept>

static IrOpcode BinaryOpcodeFor(BoundBinaryOperatorKind kind) {
  switch (kind) {
  case BoundBinaryOperatorKind::Addition:
    return IrOpcode::Add;
  case BoundBinaryOperatorKind::Subtraction:
    return IrOpcode::Sub;
  case BoundBinaryOperatorKind::Multiplication:
    return IrOpcode::Mul;
  case BoundBinaryOperatorKind::Division:
    return IrOpcode::Div;
  case BoundBinaryOperatorKind::Modulo:
    return IrOpcode::Mod;
  case BoundBinaryOperatorKind::Equals:
    return IrOpcode::Eq;
  case BoundBinaryOperatorKind::NotEquals:
    return IrOpcode::Ne;
  case BoundBinaryOperatorKind::LogicalAnd:
    return IrOpcode::And;
  case BoundBinaryOperatorKind::LogicalOr:
    return IrOpcode::Or;
  case BoundBinaryOperatorKind::BitwiseAnd:
    return IrOpcode::BitwiseAnd;
  case BoundBinaryOperatorKind::BitwiseOr:
    return IrOpcode::BitwiseOr;
  case BoundBinaryOperatorKind::BitwiseXor:
    return IrOpcode::Xor;
  case BoundBinaryOperatorKind::GreaterThan:
    return IrOpcode::Gt;
  case BoundBinaryOperatorKind::LessThan:
    return IrOpcode::Lt;
  case BoundBinaryOperatorKind::LessThanOrEqualTo:
    return IrOpcode::Le;
  case BoundBinaryOperatorKind::GreaterThanOrEqualTo:
    return IrOpcode::Ge;
  }
  return IrOpcode::Unknown;
}

static IrOpcode UnaryOpcodeFor(BoundUnaryOperatorKind kind) {
  switch (kind) {
  case BoundUnaryOperatorKind::Identity:
    return IrOpcode::Copy;
  case BoundUnaryOperatorKind::Negation:
    return IrOpcode::Neg;
  case BoundUnaryOperatorKind::LogicalNegation:
    return IrOpcode::Not;
  case BoundUnaryOperatorKind::BitwiseNot:
    return IrOpcode::BitwiseNot;
  }
  return IrOpcode::Unknown;
}

static IrOpcode AssignmentOpcodeFor(BoundAssignmentOperatorKind kind) {
  switch (kind) {
  case BoundAssignmentOperatorKind::Assignment:
    return IrOpcode::Store;
  case BoundAssignmentOperatorKind::AddAndAssign:
    return IrOpcode::StoreAdd;
  case BoundAssignmentOperatorKind::SubtractAndAssign:
    return IrOpcode::StoreSub;
  case BoundAssignmentOperatorKind::MultiplyAndAssign:
    return IrOpcode::StoreMul;
  case BoundAssignmentOperatorKind::DivideAndAssign:
    return IrOpcode::StoreDiv;
  case BoundAssignmentOperatorKind::ModuloAndAssign:
    return IrOpcode::StoreMod;
  case BoundAssignmentOperatorKind::BitwiseAndAndAssign:
    return IrOpcode::StoreBitwiseAnd;
  case BoundAssignmentOperatorKind::BitwiseOrAndAssign:
    return IrOpcode::StoreBitwiseOr;
  case BoundAssignmentOperatorKind::BitwiseXorAndAssign:
    return IrOpcode::StoreXor;
  }
  return IrOpcode::Unknown;
}

static std::string ValueText(Value value) {
  if (value.isString()) {
    return EscapeIrString(value.asString());
  }
  std::stringstream stream;
  stream << value;
  return stream.str();
}

IrModule IrLowerer::Lower(BoundStatementNode *root) {
  mModule = IrModule();
  mEmittedFunctions.clear();
  mLoopStack.clear();
  mTempIndex = 0;
  mLabelIndex = 0;
  mVariableNames.clear();
  mVariableNameCounts.clear();
  mModule.functions.push_back(IrFunction{kIrMainFunctionName, {}, {}, ""});
  mCurrentFunction = &mModule.functions.back();
  LowerStatement(root);
  if (mCurrentFunction->returnValue.empty()) {
    mCurrentFunction->returnValue = kIrVoidValue;
  }
  return std::move(mModule);
}

std::string IrLowerer::GetVariableName(const VariableSymbol *variable,
                                       const std::string &fallbackName) {
  if (!variable) {
    return fallbackName.empty() ? "%unknown" : "%" + fallbackName;
  }
  auto it = mVariableNames.find(variable);
  if (it != mVariableNames.end()) {
    return it->second;
  }
  const auto &baseName =
      variable->Name().empty() ? fallbackName : variable->Name();
  if (baseName.empty()) {
    return "%unknown";
  }
  auto &count = mVariableNameCounts[baseName];
  std::string name = (count == 0)
                         ? ("%" + baseName)
                         : ("%" + baseName + "." + std::to_string(count));
  count++;
  mVariableNames[variable] = name;
  return name;
}

void IrLowerer::Emit(IrOpcode op, std::vector<std::string> operands,
                     std::string result) {
  mCurrentFunction->instructions.push_back(
      {std::move(result), op, std::move(operands)});
}

void IrLowerer::EmitAt(size_t index, IrOpcode op,
                       std::vector<std::string> operands, std::string result) {
  mCurrentFunction->instructions.insert(
      mCurrentFunction->instructions.begin() +
          static_cast<std::ptrdiff_t>(index),
      {std::move(result), op, std::move(operands)});
}

void IrLowerer::EmitLabel(const std::string &label) {
  Emit(IrOpcode::Label, {label});
}

void IrLowerer::EmitBranch(const std::string &targetLabel) {
  Emit(IrOpcode::Br, {targetLabel});
}

void IrLowerer::EmitCondBranch(const std::string &condition,
                               const std::string &trueLabel,
                               const std::string &falseLabel) {
  Emit(IrOpcode::BrCond, {condition, trueLabel, falseLabel});
}

void IrLowerer::LowerLoop(IrLabelKind condKind, IrLabelKind bodyKind,
                          IrLabelKind endKind, BoundStatementNode *initializer,
                          BoundExpressionNode *condition,
                          BoundStatementNode *body,
                          BoundExpressionNode *increment) {
  auto id = mLabelIndex++;
  auto condLabel = MakeIrLabel(condKind, id);
  auto bodyLabel = MakeIrLabel(bodyKind, id);
  auto endLabel = MakeIrLabel(endKind, id);
  auto continueLabel =
      increment ? MakeIrLabel(IrLabelKind::ForInc, id) : condLabel;

  if (initializer) {
    LowerStatement(initializer);
  }
  EmitBranch(condLabel);
  EmitLabel(condLabel);
  auto condValue = LowerExpression(condition);
  EmitCondBranch(condValue, bodyLabel, endLabel);

  EmitLabel(bodyLabel);
  mLoopStack.push_back({endLabel, continueLabel, false});
  LowerStatement(body);
  bool usedContinue = mLoopStack.back().usedContinue;
  mLoopStack.pop_back();
  if (increment) {
    if (usedContinue) {
      EmitBranch(continueLabel);
      EmitLabel(continueLabel);
    }
    LowerExpression(increment);
  }
  EmitBranch(condLabel);
  EmitLabel(endLabel);
}

void IrLowerer::LowerDoWhile(BoundStatementNode *body,
                             BoundExpressionNode *condition) {
  auto id = mLabelIndex++;
  auto bodyLabel = MakeIrLabel(IrLabelKind::DoBody, id);
  auto condLabel = MakeIrLabel(IrLabelKind::DoCond, id);
  auto endLabel = MakeIrLabel(IrLabelKind::DoEnd, id);

  EmitBranch(bodyLabel);
  EmitLabel(bodyLabel);
  mLoopStack.push_back({endLabel, condLabel, false});
  LowerStatement(body);
  bool usedContinue = mLoopStack.back().usedContinue;
  mLoopStack.pop_back();
  if (usedContinue) {
    EmitBranch(condLabel);
    EmitLabel(condLabel);
  }
  auto condValue = LowerExpression(condition);
  EmitCondBranch(condValue, bodyLabel, endLabel);
  EmitLabel(endLabel);
}

void IrLowerer::LowerStatement(BoundStatementNode *node) {
  if (auto block = dynamic_cast<BoundBlockStatementNode *>(node)) {
    for (const auto &statement : block->Statements()) {
      LowerStatement(statement.get());
    }
    return;
  }
  if (dynamic_cast<BoundBreakStatementNode *>(node)) {
    if (!mLoopStack.empty()) {
      EmitBranch(mLoopStack.back().breakLabel);
    }
    return;
  }
  if (dynamic_cast<BoundContinueStatementNode *>(node)) {
    if (!mLoopStack.empty()) {
      mLoopStack.back().usedContinue = true;
      EmitBranch(mLoopStack.back().continueLabel);
    }
    return;
  }
  if (auto returnStatement = dynamic_cast<BoundReturnStatementNode *>(node)) {
    std::string value = kIrVoidValue;
    if (returnStatement->Expression()) {
      value = LowerExpression(
          const_cast<BoundExpressionNode *>(returnStatement->Expression()));
    }
    Emit(IrOpcode::Ret, {value});
    mCurrentFunction->returnValue = value;
    return;
  }
  if (auto expression = dynamic_cast<BoundExpressionStatementNode *>(node)) {
    mCurrentFunction->returnValue = LowerExpression(
        const_cast<BoundExpressionNode *>(expression->Expression()));
    return;
  }
  if (auto declaration = dynamic_cast<BoundVariableDeclarationNode *>(node)) {
    auto value = LowerExpression(
        const_cast<BoundExpressionNode *>(declaration->Initializer()));
    auto varName = GetVariableName(declaration->Variable().get());
    Emit(IrOpcode::Store, {value}, varName);
    mCurrentFunction->returnValue = varName;
    return;
  }
  if (dynamic_cast<BoundFunctionDeclarationNode *>(node)) {
    return;
  }
  if (auto ifStatement = dynamic_cast<BoundIfStatementNode *>(node)) {
    auto id = mLabelIndex++;
    auto thenLabel = MakeIrLabel(IrLabelKind::Then, id);
    auto elseLabel = MakeIrLabel(IrLabelKind::Else, id);
    auto endLabel = MakeIrLabel(IrLabelKind::End, id);

    auto condition = LowerExpression(
        const_cast<BoundExpressionNode *>(ifStatement->Condition()));
    auto branchIndex = mCurrentFunction->instructions.size();
    EmitCondBranch(condition, thenLabel,
                   ifStatement->ElseStatement() ? elseLabel : endLabel);

    auto prevReturn = mCurrentFunction->returnValue;
    mCurrentFunction->returnValue.clear();
    EmitLabel(thenLabel);
    LowerStatement(
        const_cast<BoundStatementNode *>(ifStatement->ThenStatement()));
    auto thenReturn = mCurrentFunction->returnValue;
    bool thenTerminated =
        !mCurrentFunction->instructions.empty() &&
        IsTerminatorOp(mCurrentFunction->instructions.back().op);
    auto thenJumpIndex = mCurrentFunction->instructions.size();
    if (!thenTerminated) {
      EmitBranch(endLabel);
    }

    std::string elseReturn;
    size_t elseJumpIndex = 0;
    bool elseTerminated = false;
    if (ifStatement->ElseStatement()) {
      mCurrentFunction->returnValue.clear();
      EmitLabel(elseLabel);
      LowerStatement(
          const_cast<BoundStatementNode *>(ifStatement->ElseStatement()));
      elseReturn = mCurrentFunction->returnValue;
      elseTerminated = !mCurrentFunction->instructions.empty() &&
                       IsTerminatorOp(mCurrentFunction->instructions.back().op);
      elseJumpIndex = mCurrentFunction->instructions.size();
      if (!elseTerminated) {
        EmitBranch(endLabel);
      }
    }

    if (!thenTerminated ||
        (ifStatement->ElseStatement() ? !elseTerminated : true)) {
      EmitLabel(endLabel);
    }
    if (ifStatement->ElseStatement()) {
      if (thenTerminated && !elseTerminated) {
        mCurrentFunction->returnValue =
            elseReturn.empty() ? prevReturn : elseReturn;
      } else if (!thenTerminated && elseTerminated) {
        mCurrentFunction->returnValue =
            thenReturn.empty() ? prevReturn : thenReturn;
      } else if (thenReturn == elseReturn) {
        mCurrentFunction->returnValue =
            thenReturn.empty() ? prevReturn : thenReturn;
      } else if (!thenReturn.empty() && !elseReturn.empty() &&
                 !thenTerminated && !elseTerminated) {
        auto resultTemp = NewTemp();
        EmitAt(elseJumpIndex, IrOpcode::Store, {elseReturn}, resultTemp);
        EmitAt(thenJumpIndex, IrOpcode::Store, {thenReturn}, resultTemp);
        mCurrentFunction->returnValue = resultTemp;
      } else {
        mCurrentFunction->returnValue =
            !thenReturn.empty() ? thenReturn : elseReturn;
      }
    } else if (!prevReturn.empty()) {
      mCurrentFunction->returnValue = prevReturn;
    } else if (!thenReturn.empty() && !thenTerminated) {
      auto resultTemp = NewTemp();
      EmitAt(thenJumpIndex, IrOpcode::Store, {thenReturn}, resultTemp);
      EmitAt(branchIndex, IrOpcode::Const, {"0"}, resultTemp);
      mCurrentFunction->returnValue = resultTemp;
    }
    return;
  }
  if (auto whileStatement = dynamic_cast<BoundWhileStatementNode *>(node)) {
    LowerLoop(
        IrLabelKind::WhileCond, IrLabelKind::WhileBody, IrLabelKind::WhileEnd,
        nullptr, const_cast<BoundExpressionNode *>(whileStatement->Condition()),
        const_cast<BoundStatementNode *>(whileStatement->Body()), nullptr);
    return;
  }
  if (auto doWhileStatement = dynamic_cast<BoundDoWhileStatementNode *>(node)) {
    LowerDoWhile(
        const_cast<BoundStatementNode *>(doWhileStatement->Body()),
        const_cast<BoundExpressionNode *>(doWhileStatement->Condition()));
    return;
  }
  if (auto forStatement = dynamic_cast<BoundForStatementNode *>(node)) {
    LowerLoop(IrLabelKind::ForCond, IrLabelKind::ForBody, IrLabelKind::ForEnd,
              const_cast<BoundStatementNode *>(forStatement->Initializer()),
              const_cast<BoundExpressionNode *>(forStatement->Condition()),
              const_cast<BoundStatementNode *>(forStatement->Body()),
              const_cast<BoundExpressionNode *>(forStatement->Increment()));
    return;
  }
  Emit(IrOpcode::UnsupportedStatement, {});
}

std::string IrLowerer::LowerExpression(BoundExpressionNode *node) {
  if (auto literal = dynamic_cast<BoundLiteralExpressionNode *>(node)) {
    auto result = NewTemp();
    Emit(IrOpcode::Const, {ValueText(literal->GetValue())}, result);
    return result;
  }
  if (auto identifier = dynamic_cast<BoundIdentifierExpressionNode *>(node)) {
    return GetVariableName(identifier->Variable().get(), identifier->Name());
  }
  if (auto unary = dynamic_cast<BoundUnaryExpressionNode *>(node)) {
    auto operand = LowerExpression(unary->Operand());
    auto result = NewTemp();
    Emit(UnaryOpcodeFor(unary->OperatorKind()), {operand}, result);
    return result;
  }
  if (auto binary = dynamic_cast<BoundBinaryExpressionNode *>(node)) {
    if (binary->OperatorKind() == BoundBinaryOperatorKind::LogicalAnd ||
        binary->OperatorKind() == BoundBinaryOperatorKind::LogicalOr) {
      auto id = mLabelIndex++;
      auto rhsLabel = MakeIrLabel(IrLabelKind::LogicRhs, id);
      auto endLabel = MakeIrLabel(IrLabelKind::LogicEnd, id);
      auto left = LowerExpression(binary->Left());
      auto result = NewTemp();
      Emit(IrOpcode::Store, {left}, result);
      if (binary->OperatorKind() == BoundBinaryOperatorKind::LogicalAnd) {
        EmitCondBranch(left, rhsLabel, endLabel);
      } else {
        EmitCondBranch(left, endLabel, rhsLabel);
      }
      EmitLabel(rhsLabel);
      auto right = LowerExpression(binary->Right());
      Emit(IrOpcode::Store, {right}, result);
      EmitBranch(endLabel);
      EmitLabel(endLabel);
      return result;
    }
    auto left = LowerExpression(binary->Left());
    auto right = LowerExpression(binary->Right());
    auto result = NewTemp();
    Emit(BinaryOpcodeFor(binary->OperatorKind()), {left, right}, result);
    return result;
  }
  if (auto assignment = dynamic_cast<BoundAssignmentExpressionNode *>(node)) {
    auto value = LowerExpression(assignment->BoundExpression());
    auto result =
        GetVariableName(assignment->Variable().get(), assignment->Identifier());
    Emit(AssignmentOpcodeFor(assignment->OperatorKind()), {value}, result);
    return result;
  }
  if (auto call = dynamic_cast<BoundCallExpressionNode *>(node)) {
    if (call->Name() == "print" && !call->Body() &&
        mEmittedFunctions.find("print") == mEmittedFunctions.end()) {
      auto arg = LowerExpression(call->Arguments()[0].get());
      auto result = NewTemp();
      Emit(IrOpcode::Print, {arg}, result);
      return result;
    }
    LowerFunction(call);
    auto arguments = std::vector<std::string>{call->Name()};
    for (const auto &argument : call->Arguments()) {
      arguments.push_back(LowerExpression(argument.get()));
    }
    auto result = NewTemp();
    Emit(IrOpcode::Call, std::move(arguments), result);
    return result;
  }
  auto result = NewTemp();
  Emit(IrOpcode::Unknown, {}, result);
  return result;
}

void IrLowerer::LowerFunction(BoundCallExpressionNode *call) {
  if (!call->Body() ||
      mEmittedFunctions.find(call->Name()) != mEmittedFunctions.end()) {
    return;
  }

  mEmittedFunctions.insert(call->Name());
  auto savedFunctionIndex =
      static_cast<size_t>(mCurrentFunction - mModule.functions.data());
  auto savedTempIndex = mTempIndex;
  auto savedLabelIndex = mLabelIndex;
  auto savedLoopStack = std::move(mLoopStack);
  auto savedVariableNames = std::move(mVariableNames);
  auto savedVariableNameCounts = std::move(mVariableNameCounts);
  mTempIndex = 0;
  mLabelIndex = 0;
  mLoopStack.clear();
  mVariableNames.clear();
  mVariableNameCounts.clear();

  auto parameters = std::vector<std::string>();
  for (const auto &parameter : call->Parameters()) {
    parameters.push_back(
        GetVariableName(parameter.get(), parameter ? parameter->Name() : ""));
  }

  mModule.functions.push_back(IrFunction{call->Name(), parameters, {}, ""});
  mCurrentFunction = &mModule.functions.back();
  LowerStatement(const_cast<BoundStatementNode *>(call->Body()));
  if (mCurrentFunction->returnValue.empty()) {
    mCurrentFunction->returnValue = kIrVoidValue;
  }

  mCurrentFunction = &mModule.functions[savedFunctionIndex];
  mTempIndex = savedTempIndex;
  mLabelIndex = savedLabelIndex;
  mLoopStack = std::move(savedLoopStack);
  mVariableNames = std::move(savedVariableNames);
  mVariableNameCounts = std::move(savedVariableNameCounts);
}

std::string IrLowerer::NewTemp() { return "%" + std::to_string(mTempIndex++); }

void IrPrinter::Print(const IrModule &module, std::ostream &out) {
  for (size_t i = 0; i < module.functions.size(); i++) {
    const auto &function = module.functions[i];
    out << "func " << function.name << "(";
    for (size_t j = 0; j < function.parameters.size(); j++) {
      if (j != 0) {
        out << ", ";
      }
      out << function.parameters[j];
    }
    out << ")" << std::endl;
    out << IrLabelPrefix(IrLabelKind::Entry) << ":" << std::endl;
    for (const auto &instruction : function.instructions) {
      if (instruction.op == IrOpcode::Label) {
        out << instruction.operands[0] << ":" << std::endl;
        continue;
      }
      out << "  ";
      if (!instruction.result.empty()) {
        out << instruction.result << " = ";
      }
      out << IrOpcodeToString(instruction.op);
      for (const auto &operand : instruction.operands) {
        out << " " << operand;
      }
      out << std::endl;
    }
    if (function.instructions.empty() ||
        function.instructions.back().op != IrOpcode::Ret) {
      out << "  ret " << function.returnValue << std::endl;
    }
    if (i + 1 < module.functions.size()) {
      out << std::endl;
    }
  }
}

IrModule IrParser::Parse(const std::string &text) {
  auto module = IrModule();
  IrFunction *currentFunction = nullptr;
  const std::string entryHeader = IrLabelPrefix(IrLabelKind::Entry) + ":";

  std::stringstream stream(text);
  std::string line;
  while (std::getline(stream, line)) {
    line = Trim(line);
    if (line.empty() || line == entryHeader) {
      continue;
    }
    if (line.rfind("func ", 0) == 0) {
      auto open = line.find('(');
      auto close = line.find(')', open == std::string::npos ? 0 : open);
      if (open == std::string::npos || close == std::string::npos) {
        throw std::runtime_error("IR parse error: malformed function header.");
      }
      auto name = Trim(line.substr(5, open - 5));
      auto parameters =
          ParseParameters(line.substr(open + 1, close - open - 1));
      module.functions.push_back(IrFunction{name, parameters, {}, ""});
      currentFunction = &module.functions.back();
      continue;
    }

    if (!currentFunction) {
      throw std::runtime_error("IR parse error: instruction before function.");
    }

    if (line.back() == ':') {
      auto labelName = Trim(line.substr(0, line.size() - 1));
      currentFunction->instructions.push_back(
          {"", IrOpcode::Label, {labelName}});
      continue;
    }

    if (line.rfind("ret ", 0) == 0) {
      auto retVal = Trim(line.substr(4));
      currentFunction->instructions.push_back({"", IrOpcode::Ret, {retVal}});
      currentFunction->returnValue = retVal;
      continue;
    }

    std::string result;
    std::string rhs = line;
    auto equals = line.find(" = ");
    if (equals != std::string::npos) {
      result = Trim(line.substr(0, equals));
      rhs = line.substr(equals + 3);
    }

    auto parts = SplitWhitespace(rhs);
    if (parts.empty()) {
      if (equals != std::string::npos) {
        throw std::runtime_error("IR parse error: missing instruction op.");
      }
      continue;
    }

    currentFunction->instructions.push_back(
        {result, IrOpcodeFromString(parts[0]),
         std::vector<std::string>(parts.begin() + 1, parts.end())});
  }

  return module;
}

std::string IrParser::Trim(const std::string &text) {
  constexpr const char *kWhitespace = " \t\r\n\v\f";
  auto first = text.find_first_not_of(kWhitespace);
  if (first == std::string::npos) {
    return "";
  }
  auto last = text.find_last_not_of(kWhitespace);
  return text.substr(first, last - first + 1);
}

std::vector<std::string> IrParser::SplitWhitespace(const std::string &text) {
  std::vector<std::string> result;
  size_t i = 0;
  size_t n = text.size();
  while (i < n) {
    while (i < n && std::isspace(static_cast<unsigned char>(text[i]))) {
      i++;
    }
    if (i >= n) {
      break;
    }
    if (text[i] == '"') {
      size_t start = i++;
      bool closed = false;
      while (i < n) {
        if (text[i] == '\\') {
          i++;
          if (i < n) {
            i++;
          }
        } else if (text[i] == '"') {
          i++;
          closed = true;
          break;
        } else {
          i++;
        }
      }
      if (!closed) {
        throw std::runtime_error(
            "IR parse error: unterminated string literal.");
      }
      result.push_back(text.substr(start, i - start));
    } else {
      size_t start = i;
      while (i < n && !std::isspace(static_cast<unsigned char>(text[i]))) {
        i++;
      }
      result.push_back(text.substr(start, i - start));
    }
  }
  return result;
}

std::vector<std::string> IrParser::ParseParameters(const std::string &text) {
  auto parameters = std::vector<std::string>();
  std::stringstream stream(text);
  std::string parameter;
  while (std::getline(stream, parameter, ',')) {
    parameter = Trim(parameter);
    if (!parameter.empty()) {
      parameters.push_back(parameter);
    }
  }
  return parameters;
}

Value IrInterpreter::Execute(const IrModule &module) {
  mModule = &module;
  auto mainFunction = FindFunction(kIrMainFunctionName);
  if (!mainFunction) {
    throw std::runtime_error("IR error: missing __main function.");
  }
  return ExecuteFunction(*mainFunction, {});
}

Value IrInterpreter::EvaluateUnaryOp(IrOpcode op, Value operand) {
  switch (op) {
  case IrOpcode::Copy:
    return operand;
  case IrOpcode::Neg:
    return -operand;
  case IrOpcode::Not:
    return !operand;
  case IrOpcode::BitwiseNot:
    return ~operand;
  default:
    throw std::runtime_error("IR error: unknown unary instruction " +
                             IrOpcodeToString(op) + ".");
  }
}

Value IrInterpreter::EvaluateBinaryOp(IrOpcode op, Value left, Value right) {
  switch (op) {
  case IrOpcode::Add:
    return left + right;
  case IrOpcode::Sub:
    return left - right;
  case IrOpcode::Mul:
    return left * right;
  case IrOpcode::Div:
    return left / right;
  case IrOpcode::Mod:
    return left % right;
  case IrOpcode::Eq:
    return left == right;
  case IrOpcode::Ne:
    return left != right;
  case IrOpcode::And:
    return left && right;
  case IrOpcode::Or:
    return left || right;
  case IrOpcode::BitwiseAnd:
    return left & right;
  case IrOpcode::BitwiseOr:
    return left | right;
  case IrOpcode::Xor:
    return left ^ right;
  case IrOpcode::Gt:
    return left > right;
  case IrOpcode::Lt:
    return left < right;
  case IrOpcode::Le:
    return left <= right;
  case IrOpcode::Ge:
    return left >= right;
  default:
    throw std::runtime_error("IR error: unknown binary instruction " +
                             IrOpcodeToString(op) + ".");
  }
}

size_t IrInterpreter::LookupLabel(
    const std::string &label,
    const std::unordered_map<std::string, size_t> &labelMap) {
  auto target = labelMap.find(label);
  if (target == labelMap.end()) {
    throw std::runtime_error("IR error: unknown label " + label + ".");
  }
  return target->second;
}

Value IrInterpreter::ExecuteFunction(const IrFunction &function,
                                     const std::vector<Value> &arguments) {
  auto values = std::unordered_map<std::string, Value>();
  if (arguments.size() != function.parameters.size()) {
    throw std::runtime_error("IR error: argument count mismatch.");
  }
  for (size_t i = 0; i < arguments.size(); i++) {
    values[function.parameters[i]] = arguments[i];
  }

  auto labelMap = std::unordered_map<std::string, size_t>();
  for (size_t i = 0; i < function.instructions.size(); i++) {
    const auto &inst = function.instructions[i];
    if (inst.op == IrOpcode::Label && !inst.operands.empty()) {
      labelMap[inst.operands[0]] = i;
    }
  }

  for (size_t ip = 0; ip < function.instructions.size(); ip++) {
    const auto &instruction = function.instructions[ip];
    auto operand = [&](size_t index) {
      return EvaluateOperand(instruction.operands[index], values);
    };

    if (instruction.op == IrOpcode::Label) {
      continue;
    }
    if (instruction.op == IrOpcode::Ret) {
      if (instruction.operands[0] == kIrVoidValue) {
        return Value();
      }
      return operand(0);
    }
    if (instruction.op == IrOpcode::Br) {
      ip = LookupLabel(instruction.operands[0], labelMap);
      continue;
    }
    if (instruction.op == IrOpcode::BrCond) {
      auto cond = operand(0);
      bool isTrue = cond.isBool() ? cond.asBool() : (cond.asInt() != 0);
      ip = LookupLabel(
          isTrue ? instruction.operands[1] : instruction.operands[2], labelMap);
      continue;
    }
    if (instruction.op == IrOpcode::Const) {
      Value value;
      if (instruction.operands[0] == kIrTrueLiteral) {
        value = true;
      } else if (instruction.operands[0] == kIrFalseLiteral) {
        value = false;
      } else if (instruction.operands[0].size() >= 2 &&
                 instruction.operands[0].front() == '"' &&
                 instruction.operands[0].back() == '"') {
        value = UnescapeIrString(instruction.operands[0]);
      } else {
        value = static_cast<int32_t>(std::stoll(instruction.operands[0]));
      }
      values[instruction.result] = value;
      continue;
    }
    if (instruction.op == IrOpcode::Store) {
      values[instruction.result] = operand(0);
      continue;
    }
    if (IsUnaryOp(instruction.op)) {
      values[instruction.result] = EvaluateUnaryOp(instruction.op, operand(0));
      continue;
    }
    if (IsBinaryOp(instruction.op)) {
      values[instruction.result] =
          EvaluateBinaryOp(instruction.op, operand(0), operand(1));
      continue;
    }
    if (IsCompoundStoreOp(instruction.op)) {
      values[instruction.result] = EvaluateBinaryOp(
          CompoundStoreToBinaryOp(instruction.op),
          EvaluateOperand(instruction.result, values), operand(0));
      continue;
    }
    if (instruction.op == IrOpcode::Print) {
      auto val = operand(0);
      std::cout << val << std::endl;
      values[instruction.result] = val;
      continue;
    }
    if (instruction.op == IrOpcode::Call) {
      auto target = FindFunction(instruction.operands[0]);
      if (!target) {
        throw std::runtime_error("IR error: unknown function " +
                                 instruction.operands[0] + ".");
      }
      auto callArguments = std::vector<Value>();
      for (size_t i = 1; i < instruction.operands.size(); i++) {
        callArguments.push_back(operand(i));
      }
      values[instruction.result] = ExecuteFunction(*target, callArguments);
      continue;
    }
    if (instruction.op == IrOpcode::UnsupportedStatement) {
      throw std::runtime_error("IR error: unsupported statement.");
    }
    throw std::runtime_error("IR error: unknown instruction " +
                             IrOpcodeToString(instruction.op) + ".");
  }

  if (function.returnValue == kIrVoidValue) {
    return Value();
  }
  return EvaluateOperand(function.returnValue, values);
}

Value IrInterpreter::EvaluateOperand(
    const std::string &operand,
    const std::unordered_map<std::string, Value> &values) {
  auto value = values.find(operand);
  if (value != values.end()) {
    return value->second;
  }
  if (operand == kIrTrueLiteral) {
    return true;
  }
  if (operand == kIrFalseLiteral) {
    return false;
  }
  if (operand.size() >= 2 && operand.front() == '"' && operand.back() == '"') {
    return UnescapeIrString(operand);
  }
  if (!operand.empty() &&
      (std::isdigit(static_cast<unsigned char>(operand[0])) ||
       (operand[0] == '-' && operand.size() > 1 &&
        std::isdigit(static_cast<unsigned char>(operand[1]))))) {
    return static_cast<int32_t>(std::stoll(operand));
  }
  throw std::runtime_error("IR error: unknown value " + operand + ".");
}

const IrFunction *IrInterpreter::FindFunction(const std::string &name) const {
  for (const auto &function : mModule->functions) {
    if (function.name == name) {
      return &function;
    }
  }
  return nullptr;
}
