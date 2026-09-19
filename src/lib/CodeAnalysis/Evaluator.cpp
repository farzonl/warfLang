// Copyright (c) 2022 F. Lotfi All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "Evaluator.h"
#include "Binding/BoundAssignmentExpressionNode.h"
#include "Binding/BoundBinaryExpressionNode.h"
#include "Binding/BoundIdentifierExpressionNode.h"
#include "Binding/BoundLiteralExpressionNode.h"
#include "Binding/BoundBlockStatementNode.h"
#include "Binding/BoundExpressionStatementNode.h"
#include "Binding/BoundVariableDeclarationNode.h"
#include "Binding/BoundUnaryExpressionNode.h"

BoundExpressionNode *Evaluator::Root() const {
  return dynamic_cast<BoundExpressionNode *>(mRoot.get());
}
Value Evaluator::Evaluate() {
  if (auto statement = dynamic_cast<BoundStatementNode *>(mRoot.get())) {
    return EvaluateStatement(statement);
  }
  return EvaluateRec(dynamic_cast<BoundExpressionNode *>(mRoot.get()));
}

Evaluator::Evaluator(std::unique_ptr<BoundExpressionNode> root)
    : mRoot(std::move(root)) {}

Evaluator::Evaluator(std::unique_ptr<BoundStatementNode> root)
    : mRoot(std::move(root)) {}

Value Evaluator::EvaluateStatement(BoundStatementNode *node) {
  if (auto block = dynamic_cast<BoundBlockStatementNode *>(node)) {
    Value result;
    for (const auto &statement : block->Statements()) {
      result = EvaluateStatement(statement.get());
    }
    return result;
  }
  if (auto expression = dynamic_cast<BoundExpressionStatementNode *>(node)) {
    return EvaluateRec(const_cast<BoundExpressionNode *>(expression->Expression()));
  }
  if (auto declaration = dynamic_cast<BoundVariableDeclarationNode *>(node)) {
    auto value = EvaluateRec(const_cast<BoundExpressionNode *>(declaration->Initializer()));
    declaration->Variable()->SetValue(value);
    return value;
  }
  throw std::runtime_error("EvaluatorError: Unexpected statement");
}

Value Evaluator::EvaluateRec(BoundExpressionNode *node) {
  if (BoundLiteralExpressionNode *literal =
          dynamic_cast<BoundLiteralExpressionNode *>(node)) {
    return literal->GetValue();
  }
  if (BoundIdentifierExpressionNode *identifierExpression =
          dynamic_cast<BoundIdentifierExpressionNode *>(node)) {
    return identifierExpression->Variable()->GetValue();
  }
  if (BoundAssignmentExpressionNode *assignmentExpression =
          dynamic_cast<BoundAssignmentExpressionNode *>(node)) {
    auto rightSide = EvaluateRec(assignmentExpression->BoundExpression());
    // TODO add assignment operator types
    auto opKind = assignmentExpression->OperatorKind();
    Value returnValue;
    switch (opKind) {
    case BoundAssignmentOperatorKind::Assignment:
      returnValue = rightSide;
      break;
    case BoundAssignmentOperatorKind::AddAndAssign:
      returnValue = assignmentExpression->Variable()->GetValue() + rightSide;
      break;
    case BoundAssignmentOperatorKind::SubtractAndAssign:
      returnValue = assignmentExpression->Variable()->GetValue() - rightSide;
      break;
    case BoundAssignmentOperatorKind::MultiplyAndAssign:
      returnValue = assignmentExpression->Variable()->GetValue() * rightSide;
      break;
    case BoundAssignmentOperatorKind::DivideAndAssign:
      returnValue = assignmentExpression->Variable()->GetValue() / rightSide;
      break;
    case BoundAssignmentOperatorKind::BitwiseAndAndAssign:
      returnValue = assignmentExpression->Variable()->GetValue() & rightSide;
      break;
    case BoundAssignmentOperatorKind::BitwiseOrAndAssign:
      returnValue = assignmentExpression->Variable()->GetValue() | rightSide;
      break;
    case BoundAssignmentOperatorKind::BitwiseXorAndAssign:
      returnValue = assignmentExpression->Variable()->GetValue() ^ rightSide;
      break;
    }
    assignmentExpression->Variable()->SetValue(returnValue);
    return returnValue;
  }
  if (BoundUnaryExpressionNode *unaryExpression =
          dynamic_cast<BoundUnaryExpressionNode *>(node)) {
    auto opKind = unaryExpression->OperatorKind();
    auto operand = EvaluateRec(unaryExpression->Operand());
    switch (opKind) {
    case BoundUnaryOperatorKind::Identity:
      return operand;
    case BoundUnaryOperatorKind::Negation:
      return -operand;
    case BoundUnaryOperatorKind::LogicalNegation:
      return !operand;
    case BoundUnaryOperatorKind::BitwiseNot:
      return ~operand;
    default:
      throw std::runtime_error("EvaluatorError: Unexpected unary operator: " +
                               BoundUnaryKindStrMap.at(opKind));
    }
  }
  if (BoundBinaryExpressionNode *binaryExpression =
          dynamic_cast<BoundBinaryExpressionNode *>(node)) {
    auto left = EvaluateRec(binaryExpression->Left());
    auto right = EvaluateRec(binaryExpression->Right());
    auto opKind = binaryExpression->OperatorKind();
    switch (opKind) {
    case BoundBinaryOperatorKind::Addition:
      return left + right;
    case BoundBinaryOperatorKind::Subtraction:
      return left - right;
    case BoundBinaryOperatorKind::Multiplication:
      return left * right;
    case BoundBinaryOperatorKind::Division:
      return left / right;
    case BoundBinaryOperatorKind::Equals:
      return left == right;
    case BoundBinaryOperatorKind::NotEquals:
      return left != right;
    case BoundBinaryOperatorKind::LogicalAnd:
      return left && right;
    case BoundBinaryOperatorKind::LogicalOr:
      return left || right;
    case BoundBinaryOperatorKind::BitwiseAnd:
      return left & right;
    case BoundBinaryOperatorKind::BitwiseOr:
      return left | right;
    case BoundBinaryOperatorKind::BitwiseXor:
      return left ^ right;
    case BoundBinaryOperatorKind::GreaterThan:
      return left > right;
    case BoundBinaryOperatorKind::GreaterThanOrEqualTo:
      return left >= right;
    case BoundBinaryOperatorKind::LessThan:
      return left < right;
    case BoundBinaryOperatorKind::LessThanOrEqualTo:
      return left <= right;
    default:
      throw std::runtime_error("EvaluatorError: Unexpected binary operator: " +
                               BoundBinaryKindStrMap.at(opKind));
      return Value();
    }
  }
  return Value();
}