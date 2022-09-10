// Copyright (c) 2022 F. Lotfi All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "Evaluator.h"
#include "Binding/BoundAssignmentExpressionNode.h"
#include "Binding/BoundBinaryExpressionNode.h"
#include "Binding/BoundIdentifierExpressionNode.h"
#include "Binding/BoundLiteralExpressionNode.h"
#include "Binding/BoundUnaryExpressionNode.h"

BoundExpressionNode *Evaluator::Root() const { return mRootExpression.get(); }
Value Evaluator::Evaluate() { return EvaluateRec(mRootExpression.get()); }

Evaluator::Evaluator(std::unique_ptr<BoundExpressionNode> root)
    : mRootExpression(std::move(root)), mRecords("Evaluator") {}

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
    auto opType = assignmentExpression->OperatorType();
    Value returnValue;
    switch (opType) {
    case BoundAssignmentOperatorType::Assignment:
      returnValue = rightSide;
      break;
    case BoundAssignmentOperatorType::AddAndAssign:
      returnValue = assignmentExpression->Variable()->GetValue() + rightSide;
      break;
    case BoundAssignmentOperatorType::SubtractAndAssign:
      returnValue = assignmentExpression->Variable()->GetValue() - rightSide;
      break;
    case BoundAssignmentOperatorType::MultiplyAndAssign:
      returnValue = assignmentExpression->Variable()->GetValue() * rightSide;
      break;
    case BoundAssignmentOperatorType::DivideAndAssign:
      returnValue = assignmentExpression->Variable()->GetValue() / rightSide;
      break;
    case BoundAssignmentOperatorType::BitwiseAndAndAssign:
      returnValue = assignmentExpression->Variable()->GetValue() & rightSide;
      break;
    case BoundAssignmentOperatorType::BitwiseOrAndAssign:
      returnValue = assignmentExpression->Variable()->GetValue() | rightSide;
      break;
    case BoundAssignmentOperatorType::BitwiseXorAndAssign:
      returnValue = assignmentExpression->Variable()->GetValue() ^ rightSide;
      break;
    }
    assignmentExpression->Variable()->SetValue(returnValue);
    return returnValue;
  }
  if (BoundUnaryExpressionNode *unaryExpression =
          dynamic_cast<BoundUnaryExpressionNode *>(node)) {
    auto opType = unaryExpression->OperatorType();
    auto operand = EvaluateRec(unaryExpression->Operand());
    switch (opType) {
    case BoundUnaryOperatorType::Identity:
      return operand;
    case BoundUnaryOperatorType::Negation:
      return -operand;
    case BoundUnaryOperatorType::LogicalNegation:
      return !operand;
    case BoundUnaryOperatorType::BitwiseNot:
      return ~operand;
    default:
      throw std::runtime_error("EvaluatorError: Unexpected unary operator: " +
                               BoundUnaryTypeStrMap.at(opType));
    }
  }
  if (BoundBinaryExpressionNode *binaryExpression =
          dynamic_cast<BoundBinaryExpressionNode *>(node)) {
    auto left = EvaluateRec(binaryExpression->Left());
    auto right = EvaluateRec(binaryExpression->Right());
    auto opType = binaryExpression->OperatorType();
    switch (opType) {
    case BoundBinaryOperatorType::Addition:
      return left + right;
    case BoundBinaryOperatorType::Subtraction:
      return left - right;
    case BoundBinaryOperatorType::Multiplication:
      return left * right;
    case BoundBinaryOperatorType::Division:
      return left / right;
    case BoundBinaryOperatorType::Equals:
      return left == right;
    case BoundBinaryOperatorType::NotEquals:
      return left != right;
    case BoundBinaryOperatorType::LogicalAnd:
      return left && right;
    case BoundBinaryOperatorType::LogicalOr:
      return left || right;
    case BoundBinaryOperatorType::BitwiseAnd:
      return left & right;
    case BoundBinaryOperatorType::BitwiseOr:
      return left | right;
    case BoundBinaryOperatorType::BitwiseXor:
      return left ^ right;
    case BoundBinaryOperatorType::GreaterThan:
      return left > right;
    case BoundBinaryOperatorType::GreaterThanOrEqualTo:
      return left >= right;
    case BoundBinaryOperatorType::LessThan:
      return left < right;
    case BoundBinaryOperatorType::LessThanOrEqualTo:
      return left <= right;
    default:
      throw std::runtime_error("EvaluatorError: Unexpected binary operator: " +
                               BoundBinaryTypeStrMap.at(opType));
      return Value();
    }
  }
  return Value();
}