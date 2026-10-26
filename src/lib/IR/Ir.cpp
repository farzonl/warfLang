// Copyright (c) 2026 F. Lotfi All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "Ir.h"
#include "Binding/BoundAssignmentExpressionNode.h"
#include "Binding/BoundBinaryExpressionNode.h"
#include "Binding/BoundBlockStatementNode.h"
#include "Binding/BoundCallExpressionNode.h"
#include "Binding/BoundExpressionStatementNode.h"
#include "Binding/BoundForStatementNode.h"
#include "Binding/BoundFunctionDeclarationNode.h"
#include "Binding/BoundIdentifierExpressionNode.h"
#include "Binding/BoundIfStatementNode.h"
#include "Binding/BoundLiteralExpressionNode.h"
#include "Binding/BoundUnaryExpressionNode.h"
#include "Binding/BoundVariableDeclarationNode.h"
#include "Binding/BoundWhileStatementNode.h"
#include <algorithm>
#include <cctype>
#include <sstream>
#include <stdexcept>

static std::string BinaryOpName(BoundBinaryOperatorKind kind) {
  switch (kind) {
  case BoundBinaryOperatorKind::Addition:
    return "add";
  case BoundBinaryOperatorKind::Subtraction:
    return "sub";
  case BoundBinaryOperatorKind::Multiplication:
    return "mul";
  case BoundBinaryOperatorKind::Division:
    return "div";
  case BoundBinaryOperatorKind::Equals:
    return "eq";
  case BoundBinaryOperatorKind::NotEquals:
    return "ne";
  case BoundBinaryOperatorKind::LogicalAnd:
    return "and";
  case BoundBinaryOperatorKind::LogicalOr:
    return "or";
  case BoundBinaryOperatorKind::BitwiseAnd:
    return "band";
  case BoundBinaryOperatorKind::BitwiseOr:
    return "bor";
  case BoundBinaryOperatorKind::BitwiseXor:
    return "xor";
  case BoundBinaryOperatorKind::GreaterThan:
    return "gt";
  case BoundBinaryOperatorKind::LessThan:
    return "lt";
  case BoundBinaryOperatorKind::LessThanOrEqualTo:
    return "le";
  case BoundBinaryOperatorKind::GreaterThanOrEqualTo:
    return "ge";
  }
  return "unknown";
}

static std::string UnaryOpName(BoundUnaryOperatorKind kind) {
  switch (kind) {
  case BoundUnaryOperatorKind::Identity:
    return "copy";
  case BoundUnaryOperatorKind::Negation:
    return "neg";
  case BoundUnaryOperatorKind::LogicalNegation:
    return "not";
  case BoundUnaryOperatorKind::BitwiseNot:
    return "bnot";
  }
  return "unknown";
}

static std::string AssignmentOpName(BoundAssignmentOperatorKind kind) {
  switch (kind) {
  case BoundAssignmentOperatorKind::Assignment:
    return "store";
  case BoundAssignmentOperatorKind::AddAndAssign:
    return "store_add";
  case BoundAssignmentOperatorKind::SubtractAndAssign:
    return "store_sub";
  case BoundAssignmentOperatorKind::MultiplyAndAssign:
    return "store_mul";
  case BoundAssignmentOperatorKind::DivideAndAssign:
    return "store_div";
  case BoundAssignmentOperatorKind::BitwiseAndAndAssign:
    return "store_band";
  case BoundAssignmentOperatorKind::BitwiseOrAndAssign:
    return "store_bor";
  case BoundAssignmentOperatorKind::BitwiseXorAndAssign:
    return "store_xor";
  }
  return "store_unknown";
}

static std::string ValueText(Value value) {
  std::stringstream stream;
  stream << value;
  return stream.str();
}

IrModule IrLowerer::Lower(BoundStatementNode *root) {
  mModule = IrModule();
  mEmittedFunctions.clear();
  mTempIndex = 0;
  mLabelIndex = 0;
  mModule.functions.push_back(IrFunction{"__main", {}, {}, ""});
  mCurrentFunction = &mModule.functions.back();
  LowerStatement(root);
  if (mCurrentFunction->returnValue.empty()) {
    mCurrentFunction->returnValue = "void";
  }
  return std::move(mModule);
}

void IrLowerer::LowerStatement(BoundStatementNode *node) {
  if (auto block = dynamic_cast<BoundBlockStatementNode *>(node)) {
    for (const auto &statement : block->Statements()) {
      LowerStatement(statement.get());
    }
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
    mCurrentFunction->instructions.push_back(
        {"%" + declaration->Variable()->Name(), "store", {value}});
    mCurrentFunction->returnValue = "%" + declaration->Variable()->Name();
    return;
  }
  if (dynamic_cast<BoundFunctionDeclarationNode *>(node)) {
    return;
  }
  if (auto ifStatement = dynamic_cast<BoundIfStatementNode *>(node)) {
    auto id = mLabelIndex++;
    auto thenLabel = "then" + std::to_string(id);
    auto elseLabel = "else" + std::to_string(id);
    auto endLabel = "end" + std::to_string(id);

    auto condition = LowerExpression(
        const_cast<BoundExpressionNode *>(ifStatement->Condition()));
    auto branchIndex = mCurrentFunction->instructions.size();
    if (ifStatement->ElseStatement()) {
      mCurrentFunction->instructions.push_back(
          {"", "br_cond", {condition, thenLabel, elseLabel}});
    } else {
      mCurrentFunction->instructions.push_back(
          {"", "br_cond", {condition, thenLabel, endLabel}});
    }

    auto prevReturn = mCurrentFunction->returnValue;
    mCurrentFunction->returnValue.clear();
    mCurrentFunction->instructions.push_back({"", "label", {thenLabel}});
    LowerStatement(
        const_cast<BoundStatementNode *>(ifStatement->ThenStatement()));
    auto thenReturn = mCurrentFunction->returnValue;
    auto thenJumpIndex = mCurrentFunction->instructions.size();
    mCurrentFunction->instructions.push_back({"", "br", {endLabel}});

    std::string elseReturn;
    size_t elseJumpIndex = 0;
    if (ifStatement->ElseStatement()) {
      mCurrentFunction->returnValue.clear();
      mCurrentFunction->instructions.push_back({"", "label", {elseLabel}});
      LowerStatement(
          const_cast<BoundStatementNode *>(ifStatement->ElseStatement()));
      elseReturn = mCurrentFunction->returnValue;
      elseJumpIndex = mCurrentFunction->instructions.size();
      mCurrentFunction->instructions.push_back({"", "br", {endLabel}});
    }

    mCurrentFunction->instructions.push_back({"", "label", {endLabel}});
    if (ifStatement->ElseStatement()) {
      if (thenReturn == elseReturn) {
        mCurrentFunction->returnValue =
            thenReturn.empty() ? prevReturn : thenReturn;
      } else if (!thenReturn.empty() && !elseReturn.empty()) {
        auto resultTemp = NewTemp();
        mCurrentFunction->instructions.insert(
            mCurrentFunction->instructions.begin() +
                static_cast<std::ptrdiff_t>(elseJumpIndex),
            {resultTemp, "store", {elseReturn}});
        mCurrentFunction->instructions.insert(
            mCurrentFunction->instructions.begin() +
                static_cast<std::ptrdiff_t>(thenJumpIndex),
            {resultTemp, "store", {thenReturn}});
        mCurrentFunction->returnValue = resultTemp;
      } else {
        mCurrentFunction->returnValue =
            !thenReturn.empty() ? thenReturn : elseReturn;
      }
    } else {
      if (!prevReturn.empty()) {
        mCurrentFunction->returnValue = prevReturn;
      } else if (!thenReturn.empty()) {
        auto resultTemp = NewTemp();
        mCurrentFunction->instructions.insert(
            mCurrentFunction->instructions.begin() +
                static_cast<std::ptrdiff_t>(thenJumpIndex),
            {resultTemp, "store", {thenReturn}});
        mCurrentFunction->instructions.insert(
            mCurrentFunction->instructions.begin() +
                static_cast<std::ptrdiff_t>(branchIndex),
            {resultTemp, "const", {"0"}});
        mCurrentFunction->returnValue = resultTemp;
      }
    }
    return;
  }
  if (auto whileStatement = dynamic_cast<BoundWhileStatementNode *>(node)) {
    auto id = mLabelIndex++;
    auto condLabel = "while_cond" + std::to_string(id);
    auto bodyLabel = "while_body" + std::to_string(id);
    auto endLabel = "while_end" + std::to_string(id);

    mCurrentFunction->instructions.push_back({"", "br", {condLabel}});
    mCurrentFunction->instructions.push_back({"", "label", {condLabel}});
    auto condition = LowerExpression(
        const_cast<BoundExpressionNode *>(whileStatement->Condition()));
    mCurrentFunction->instructions.push_back(
        {"", "br_cond", {condition, bodyLabel, endLabel}});

    mCurrentFunction->instructions.push_back({"", "label", {bodyLabel}});
    LowerStatement(const_cast<BoundStatementNode *>(whileStatement->Body()));
    mCurrentFunction->instructions.push_back({"", "br", {condLabel}});

    mCurrentFunction->instructions.push_back({"", "label", {endLabel}});
    return;
  }
  if (auto forStatement = dynamic_cast<BoundForStatementNode *>(node)) {
    auto id = mLabelIndex++;
    auto condLabel = "for_cond" + std::to_string(id);
    auto bodyLabel = "for_body" + std::to_string(id);
    auto endLabel = "for_end" + std::to_string(id);

    if (forStatement->Initializer()) {
      LowerStatement(
          const_cast<BoundStatementNode *>(forStatement->Initializer()));
    }
    mCurrentFunction->instructions.push_back({"", "br", {condLabel}});
    mCurrentFunction->instructions.push_back({"", "label", {condLabel}});
    auto condition = LowerExpression(
        const_cast<BoundExpressionNode *>(forStatement->Condition()));
    mCurrentFunction->instructions.push_back(
        {"", "br_cond", {condition, bodyLabel, endLabel}});

    mCurrentFunction->instructions.push_back({"", "label", {bodyLabel}});
    LowerStatement(const_cast<BoundStatementNode *>(forStatement->Body()));
    LowerExpression(
        const_cast<BoundExpressionNode *>(forStatement->Increment()));
    mCurrentFunction->instructions.push_back({"", "br", {condLabel}});

    mCurrentFunction->instructions.push_back({"", "label", {endLabel}});
    return;
  }
  mCurrentFunction->instructions.push_back({"", "unsupported_statement", {}});
}

std::string IrLowerer::LowerExpression(BoundExpressionNode *node) {
  if (auto literal = dynamic_cast<BoundLiteralExpressionNode *>(node)) {
    auto result = NewTemp();
    mCurrentFunction->instructions.push_back(
        {result, "const", {ValueText(literal->GetValue())}});
    return result;
  }
  if (auto identifier = dynamic_cast<BoundIdentifierExpressionNode *>(node)) {
    return "%" + identifier->Name();
  }
  if (auto unary = dynamic_cast<BoundUnaryExpressionNode *>(node)) {
    auto operand = LowerExpression(unary->Operand());
    auto result = NewTemp();
    mCurrentFunction->instructions.push_back(
        {result, UnaryOpName(unary->OperatorKind()), {operand}});
    return result;
  }
  if (auto binary = dynamic_cast<BoundBinaryExpressionNode *>(node)) {
    auto left = LowerExpression(binary->Left());
    auto right = LowerExpression(binary->Right());
    auto result = NewTemp();
    mCurrentFunction->instructions.push_back(
        {result, BinaryOpName(binary->OperatorKind()), {left, right}});
    return result;
  }
  if (auto assignment = dynamic_cast<BoundAssignmentExpressionNode *>(node)) {
    auto value = LowerExpression(assignment->BoundExpression());
    auto result = "%" + assignment->Identifier();
    mCurrentFunction->instructions.push_back(
        {result, AssignmentOpName(assignment->OperatorKind()), {value}});
    return result;
  }
  if (auto call = dynamic_cast<BoundCallExpressionNode *>(node)) {
    LowerFunction(call);
    auto arguments = std::vector<std::string>();
    arguments.push_back(call->Name());
    for (const auto &argument : call->Arguments()) {
      arguments.push_back(LowerExpression(argument.get()));
    }
    auto result = NewTemp();
    mCurrentFunction->instructions.push_back({result, "call", arguments});
    return result;
  }
  auto result = NewTemp();
  mCurrentFunction->instructions.push_back({result, "unknown", {}});
  return result;
}

void IrLowerer::LowerFunction(BoundCallExpressionNode *call) {
  if (mEmittedFunctions.find(call->Name()) != mEmittedFunctions.end()) {
    return;
  }

  mEmittedFunctions.insert(call->Name());
  auto savedFunctionIndex =
      static_cast<size_t>(mCurrentFunction - mModule.functions.data());
  auto savedTempIndex = mTempIndex;
  auto savedLabelIndex = mLabelIndex;
  mTempIndex = 0;
  mLabelIndex = 0;

  auto parameters = std::vector<std::string>();
  for (const auto &parameter : call->Parameters()) {
    parameters.push_back("%" + parameter->Name());
  }

  mModule.functions.push_back(IrFunction{call->Name(), parameters, {}, ""});
  mCurrentFunction = &mModule.functions.back();
  LowerStatement(const_cast<BoundStatementNode *>(call->Body()));
  if (mCurrentFunction->returnValue.empty()) {
    mCurrentFunction->returnValue = "void";
  }

  mCurrentFunction = &mModule.functions[savedFunctionIndex];
  mTempIndex = savedTempIndex;
  mLabelIndex = savedLabelIndex;
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
    out << "entry:" << std::endl;
    for (const auto &instruction : function.instructions) {
      if (instruction.op == "label") {
        out << instruction.operands[0] << ":" << std::endl;
        continue;
      }
      out << "  ";
      if (!instruction.result.empty()) {
        out << instruction.result << " = ";
      }
      out << instruction.op;
      for (const auto &operand : instruction.operands) {
        out << " " << operand;
      }
      out << std::endl;
    }
    out << "  ret " << function.returnValue << std::endl;
    if (i + 1 < module.functions.size()) {
      out << std::endl;
    }
  }
}

IrModule IrParser::Parse(const std::string &text) {
  auto module = IrModule();
  IrFunction *currentFunction = nullptr;

  std::stringstream stream(text);
  std::string line;
  while (std::getline(stream, line)) {
    line = Trim(line);
    if (line.empty() || line == "entry:") {
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
      currentFunction->instructions.push_back({"", "label", {labelName}});
      continue;
    }

    if (line.rfind("ret ", 0) == 0) {
      currentFunction->returnValue = Trim(line.substr(4));
      continue;
    }

    auto equals = line.find(" = ");
    if (equals == std::string::npos) {
      auto parts = SplitWhitespace(line);
      if (parts.empty()) {
        continue;
      }
      currentFunction->instructions.push_back(
          {"", parts[0],
           std::vector<std::string>(parts.begin() + 1, parts.end())});
      continue;
    }

    auto result = Trim(line.substr(0, equals));
    auto parts = SplitWhitespace(line.substr(equals + 3));
    if (parts.empty()) {
      throw std::runtime_error("IR parse error: missing instruction op.");
    }
    currentFunction->instructions.push_back(
        {result, parts[0],
         std::vector<std::string>(parts.begin() + 1, parts.end())});
  }

  return module;
}

std::string IrParser::Trim(const std::string &text) {
  auto start = std::find_if_not(text.begin(), text.end(), [](unsigned char c) {
    return std::isspace(c);
  });
  auto end = std::find_if_not(text.rbegin(), text.rend(), [](unsigned char c) {
               return std::isspace(c);
             }).base();
  if (start >= end) {
    return "";
  }
  return std::string(start, end);
}

std::vector<std::string> IrParser::SplitWhitespace(const std::string &text) {
  auto result = std::vector<std::string>();
  std::stringstream stream(text);
  std::string part;
  while (stream >> part) {
    result.push_back(part);
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
  auto mainFunction = FindFunction("__main");
  if (!mainFunction) {
    throw std::runtime_error("IR error: missing __main function.");
  }
  return ExecuteFunction(*mainFunction, {});
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
    if (inst.op == "label" && !inst.operands.empty()) {
      labelMap[inst.operands[0]] = i;
    }
  }

  for (size_t ip = 0; ip < function.instructions.size(); ip++) {
    const auto &instruction = function.instructions[ip];
    if (instruction.op == "label") {
      continue;
    } else if (instruction.op == "br") {
      auto target = labelMap.find(instruction.operands[0]);
      if (target == labelMap.end()) {
        throw std::runtime_error("IR error: unknown label " +
                                 instruction.operands[0] + ".");
      }
      ip = target->second;
      continue;
    } else if (instruction.op == "br_cond") {
      auto cond = EvaluateOperand(instruction.operands[0], values);
      bool isTrue = cond.isBool() ? cond.asBool() : (cond.asInt() != 0);
      const auto &labelName =
          isTrue ? instruction.operands[1] : instruction.operands[2];
      auto target = labelMap.find(labelName);
      if (target == labelMap.end()) {
        throw std::runtime_error("IR error: unknown label " + labelName + ".");
      }
      ip = target->second;
      continue;
    } else if (instruction.op == "const") {
      Value value;
      if (instruction.operands[0] == "true") {
        value = true;
      } else if (instruction.operands[0] == "false") {
        value = false;
      } else {
        value = std::stoi(instruction.operands[0]);
      }
      values[instruction.result] = value;
    } else if (instruction.op == "copy") {
      values[instruction.result] =
          EvaluateOperand(instruction.operands[0], values);
    } else if (instruction.op == "neg") {
      values[instruction.result] =
          -EvaluateOperand(instruction.operands[0], values);
    } else if (instruction.op == "not") {
      values[instruction.result] =
          !EvaluateOperand(instruction.operands[0], values);
    } else if (instruction.op == "bnot") {
      values[instruction.result] =
          ~EvaluateOperand(instruction.operands[0], values);
    } else if (instruction.op == "add") {
      values[instruction.result] =
          EvaluateOperand(instruction.operands[0], values) +
          EvaluateOperand(instruction.operands[1], values);
    } else if (instruction.op == "sub") {
      values[instruction.result] =
          EvaluateOperand(instruction.operands[0], values) -
          EvaluateOperand(instruction.operands[1], values);
    } else if (instruction.op == "mul") {
      values[instruction.result] =
          EvaluateOperand(instruction.operands[0], values) *
          EvaluateOperand(instruction.operands[1], values);
    } else if (instruction.op == "div") {
      values[instruction.result] =
          EvaluateOperand(instruction.operands[0], values) /
          EvaluateOperand(instruction.operands[1], values);
    } else if (instruction.op == "eq") {
      values[instruction.result] =
          EvaluateOperand(instruction.operands[0], values) ==
          EvaluateOperand(instruction.operands[1], values);
    } else if (instruction.op == "ne") {
      values[instruction.result] =
          EvaluateOperand(instruction.operands[0], values) !=
          EvaluateOperand(instruction.operands[1], values);
    } else if (instruction.op == "and") {
      values[instruction.result] =
          EvaluateOperand(instruction.operands[0], values) &&
          EvaluateOperand(instruction.operands[1], values);
    } else if (instruction.op == "or") {
      values[instruction.result] =
          EvaluateOperand(instruction.operands[0], values) ||
          EvaluateOperand(instruction.operands[1], values);
    } else if (instruction.op == "band") {
      values[instruction.result] =
          EvaluateOperand(instruction.operands[0], values) &
          EvaluateOperand(instruction.operands[1], values);
    } else if (instruction.op == "bor") {
      values[instruction.result] =
          EvaluateOperand(instruction.operands[0], values) |
          EvaluateOperand(instruction.operands[1], values);
    } else if (instruction.op == "xor") {
      values[instruction.result] =
          EvaluateOperand(instruction.operands[0], values) ^
          EvaluateOperand(instruction.operands[1], values);
    } else if (instruction.op == "gt") {
      values[instruction.result] =
          EvaluateOperand(instruction.operands[0], values) >
          EvaluateOperand(instruction.operands[1], values);
    } else if (instruction.op == "lt") {
      values[instruction.result] =
          EvaluateOperand(instruction.operands[0], values) <
          EvaluateOperand(instruction.operands[1], values);
    } else if (instruction.op == "le") {
      values[instruction.result] =
          EvaluateOperand(instruction.operands[0], values) <=
          EvaluateOperand(instruction.operands[1], values);
    } else if (instruction.op == "ge") {
      values[instruction.result] =
          EvaluateOperand(instruction.operands[0], values) >=
          EvaluateOperand(instruction.operands[1], values);
    } else if (instruction.op == "store") {
      values[instruction.result] =
          EvaluateOperand(instruction.operands[0], values);
    } else if (instruction.op == "store_add") {
      values[instruction.result] =
          EvaluateOperand(instruction.result, values) +
          EvaluateOperand(instruction.operands[0], values);
    } else if (instruction.op == "store_sub") {
      values[instruction.result] =
          EvaluateOperand(instruction.result, values) -
          EvaluateOperand(instruction.operands[0], values);
    } else if (instruction.op == "store_mul") {
      values[instruction.result] =
          EvaluateOperand(instruction.result, values) *
          EvaluateOperand(instruction.operands[0], values);
    } else if (instruction.op == "store_div") {
      values[instruction.result] =
          EvaluateOperand(instruction.result, values) /
          EvaluateOperand(instruction.operands[0], values);
    } else if (instruction.op == "store_band") {
      values[instruction.result] =
          EvaluateOperand(instruction.result, values) &
          EvaluateOperand(instruction.operands[0], values);
    } else if (instruction.op == "store_bor") {
      values[instruction.result] =
          EvaluateOperand(instruction.result, values) |
          EvaluateOperand(instruction.operands[0], values);
    } else if (instruction.op == "store_xor") {
      values[instruction.result] =
          EvaluateOperand(instruction.result, values) ^
          EvaluateOperand(instruction.operands[0], values);
    } else if (instruction.op == "call") {
      auto function = FindFunction(instruction.operands[0]);
      if (!function) {
        throw std::runtime_error("IR error: unknown function " +
                                 instruction.operands[0] + ".");
      }
      auto callArguments = std::vector<Value>();
      for (size_t i = 1; i < instruction.operands.size(); i++) {
        callArguments.push_back(
            EvaluateOperand(instruction.operands[i], values));
      }
      values[instruction.result] = ExecuteFunction(*function, callArguments);
    } else if (instruction.op == "unsupported_statement") {
      throw std::runtime_error("IR error: unsupported statement.");
    } else {
      throw std::runtime_error("IR error: unknown instruction " +
                               instruction.op + ".");
    }
  }

  if (function.returnValue == "void") {
    return Value();
  }
  return EvaluateOperand(function.returnValue, values);
}

Value IrInterpreter::EvaluateOperand(
    const std::string &operand,
    const std::unordered_map<std::string, Value> &values) {
  auto value = values.find(operand);
  if (value == values.end()) {
    throw std::runtime_error("IR error: unknown value " + operand + ".");
  }
  return value->second;
}

const IrFunction *IrInterpreter::FindFunction(const std::string &name) const {
  for (const auto &function : mModule->functions) {
    if (function.name == name) {
      return &function;
    }
  }
  return nullptr;
}
