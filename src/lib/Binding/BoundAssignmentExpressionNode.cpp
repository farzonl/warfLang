// Copyright (c) 2022 F. Lotfi All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "BoundAssignmentExpressionNode.h"

const std::shared_ptr<BoundAssignmentOperator>
    BoundAssignmentOperator::sOperators[] = {
        std::make_shared<BoundAssignmentOperator>(SyntaxKind::EqualsToken,
                                                  Value::Type::Number),
        std::make_shared<BoundAssignmentOperator>(SyntaxKind::EqualsToken,
                                                  Value::Type::Boolean),
        std::make_shared<BoundAssignmentOperator>(SyntaxKind::PlusEqualsToken,
                                                  Value::Type::Number),
        std::make_shared<BoundAssignmentOperator>(SyntaxKind::MinusEqualsToken,
                                                  Value::Type::Number),
        std::make_shared<BoundAssignmentOperator>(SyntaxKind::StarEqualsToken,
                                                  Value::Type::Number),
        std::make_shared<BoundAssignmentOperator>(SyntaxKind::SlashEqualsToken,
                                                  Value::Type::Number),
        std::make_shared<BoundAssignmentOperator>(
            SyntaxKind::AmpersandEqualsToken, Value::Type::Number),
        std::make_shared<BoundAssignmentOperator>(SyntaxKind::PipeEqualsToken,
                                                  Value::Type::Number),
        std::make_shared<BoundAssignmentOperator>(SyntaxKind::HatEqualsToken,
                                                  Value::Type::Number),
};

const std::shared_ptr<BoundAssignmentOperator>
BoundAssignmentOperator::Bind(SyntaxKind syntaxKind, Value::Type rhsOperandType) {
  for (std::shared_ptr<BoundAssignmentOperator> op :
       BoundAssignmentOperator::sOperators) {
    if (op->GetSyntaxKind() == syntaxKind &&
        op->RightHandExpressionType() == rhsOperandType) {
      return op;
    }
  }
  throw std::runtime_error("Failed to Bind AssignmentExpression");
}

BoundAssignmentOperatorKind BoundAssignmentOperator::AssignmentKind() {
  return mAssigmentKind;
}

void BoundAssignmentOperator::setOperatorKind() {
  switch (mSyntaxKind.GetValue()) {
  case SyntaxKind::EqualsToken:
    mAssigmentKind = BoundAssignmentOperatorKind::Assignment;
    break;
  case SyntaxKind::PlusEqualsToken:
    mAssigmentKind = BoundAssignmentOperatorKind::AddAndAssign;
    break;
  case SyntaxKind::MinusEqualsToken:
    mAssigmentKind = BoundAssignmentOperatorKind::SubtractAndAssign;
    break;
  case SyntaxKind::StarEqualsToken:
    mAssigmentKind = BoundAssignmentOperatorKind::MultiplyAndAssign;
    break;
  case SyntaxKind::SlashEqualsToken:
    mAssigmentKind = BoundAssignmentOperatorKind::DivideAndAssign;
    break;
  case SyntaxKind::AmpersandEqualsToken:
    mAssigmentKind = BoundAssignmentOperatorKind::BitwiseAndAndAssign;
    break;
  case SyntaxKind::PipeEqualsToken:
    mAssigmentKind = BoundAssignmentOperatorKind::BitwiseOrAndAssign;
    break;
  case SyntaxKind::HatEqualsToken:
    mAssigmentKind = BoundAssignmentOperatorKind::BitwiseXorAndAssign;
    break;
  default:
    throw std::runtime_error(
        "Unexpected SyntaxKind seen while processing a BoundAssignment");
  }
}

BoundAssignmentOperator::BoundAssignmentOperator(SyntaxKind syntaxKind,
                                                 Value::Type rhsOperandType)
    : mSyntaxKind(syntaxKind), mRhsType(rhsOperandType) {
  setOperatorKind();
}

SyntaxKind BoundAssignmentOperator::GetSyntaxKind() { return mSyntaxKind; }
Value::Type BoundAssignmentOperator::RightHandExpressionType() { return mRhsType; }

BoundNodeKind BoundAssignmentExpressionNode::NodeKind() {
  return BoundNodeKind::AssignmentExpression;
}

Value::Type BoundAssignmentExpressionNode::Type() {
  return mBoundExpression->Type();
}

BoundExpressionNode *BoundAssignmentExpressionNode::BoundExpression() {
  return mBoundExpression.get();
}
std::string BoundAssignmentExpressionNode::Identifier() {
  return mVariable->Name();
}

std::shared_ptr<VariableSymbol> BoundAssignmentExpressionNode::Variable() {
  return mVariable;
}

BoundAssignmentOperatorKind BoundAssignmentExpressionNode::OperatorKind() {
  return mAssignmentOperator->AssignmentKind();
}

BoundAssignmentExpressionNode::BoundAssignmentExpressionNode(
    std::shared_ptr<VariableSymbol> variable,
    std::unique_ptr<BoundExpressionNode> boundExpression,
    std::shared_ptr<BoundAssignmentOperator> assignmentOperator)
    : BoundExpressionNode(), mVariable(variable),
      mBoundExpression(std::move(boundExpression)),
      mAssignmentOperator(assignmentOperator) {}