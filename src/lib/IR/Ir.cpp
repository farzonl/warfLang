// Copyright (c) 2026 F. Lotfi All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "Ir.h"
#include "Binding/BoundAssignmentExpressionNode.h"
#include "Binding/BoundBinaryExpressionNode.h"
#include "Binding/BoundBlockStatementNode.h"
#include "Binding/BoundCallExpressionNode.h"
#include "Binding/BoundExpressionStatementNode.h"
#include "Binding/BoundFunctionDeclarationNode.h"
#include "Binding/BoundIdentifierExpressionNode.h"
#include "Binding/BoundLiteralExpressionNode.h"
#include "Binding/BoundUnaryExpressionNode.h"
#include "Binding/BoundVariableDeclarationNode.h"
#include <sstream>

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
  mTempIndex = 0;

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