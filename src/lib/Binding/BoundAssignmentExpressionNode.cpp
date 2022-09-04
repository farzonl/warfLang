// Copyright (c) 2022 F. Lotfi All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "BoundAssignmentExpressionNode.h"

const std::shared_ptr<BoundAssignmentOperator>
    BoundAssignmentOperator::sOperators[] = {
        std::make_shared<BoundAssignmentOperator>(SyntaxType::EqualsToken,
                                                  Type::Number),
        std::make_shared<BoundAssignmentOperator>(SyntaxType::EqualsToken,
                                                  Type::Boolean),
        std::make_shared<BoundAssignmentOperator>(SyntaxType::PlusEqualsToken,
                                                  Type::Number),
        std::make_shared<BoundAssignmentOperator>(SyntaxType::MinusEqualsToken,
                                                  Type::Number),
        std::make_shared<BoundAssignmentOperator>(SyntaxType::StarEqualsToken,
                                                  Type::Number),
        std::make_shared<BoundAssignmentOperator>(SyntaxType::SlashEqualsToken,
                                                  Type::Number),
        std::make_shared<BoundAssignmentOperator>(
            SyntaxType::AmpersandEqualsToken, Type::Number),
        std::make_shared<BoundAssignmentOperator>(SyntaxType::PipeEqualsToken,
                                                  Type::Number),
        std::make_shared<BoundAssignmentOperator>(SyntaxType::HatEqualsToken,
                                                  Type::Number),
};

const std::shared_ptr<BoundAssignmentOperator>
BoundAssignmentOperator::Bind(SyntaxType syntaxType, Type rhsOperandType) {
  for (std::shared_ptr<BoundAssignmentOperator> op :
       BoundAssignmentOperator::sOperators) {
    if (op->GetSyntaxType() == syntaxType &&
        op->RightHandExpressionType() == rhsOperandType) {
      return op;
    }
  }
  throw std::runtime_error("Failed to Bind AssignmentExpression");
}

BoundAssignmentOperatorType BoundAssignmentOperator::AssignmentType() {
  return mAssigmentType;
}

void BoundAssignmentOperator::setOperatorType() {
  switch(mSyntaxType.GetValue()) {
    case SyntaxType::EqualsToken:
    mAssigmentType = BoundAssignmentOperatorType::Assignment;
    break;
    case SyntaxType::PlusEqualsToken:
    mAssigmentType = BoundAssignmentOperatorType::AddAndAssign;
    break;
    case SyntaxType::MinusEqualsToken:
    mAssigmentType = BoundAssignmentOperatorType::SubtractAndAssign;
    break;
    case SyntaxType::StarEqualsToken:
    mAssigmentType = BoundAssignmentOperatorType::MultiplyAndAssign;
    break;
    case SyntaxType::SlashEqualsToken:
    mAssigmentType = BoundAssignmentOperatorType::DivideAndAssign;
    break;
    case SyntaxType::AmpersandEqualsToken:
    mAssigmentType = BoundAssignmentOperatorType::BitwiseAndAndAssign;
    break;
    case SyntaxType::PipeEqualsToken:
    mAssigmentType = BoundAssignmentOperatorType::BitwiseOrAndAssign;
    break;
    case SyntaxType::HatEqualsToken:
    mAssigmentType = BoundAssignmentOperatorType::BitwiseXorAndAssign;
    break;
    default:
      throw std::runtime_error("Unexpected SyntaxType aseen while processing a BoundAssignment");
  }
}


BoundAssignmentOperator::BoundAssignmentOperator(SyntaxType syntaxType,
                                                 Type rhsOperandType)
    : mSyntaxType(syntaxType), mRhsType(rhsOperandType) {
      setOperatorType();
    }

SyntaxType BoundAssignmentOperator::GetSyntaxType() { return mSyntaxType; }
Type BoundAssignmentOperator::RightHandExpressionType() { return mRhsType; }

BoundNodeType BoundAssignmentExpressionNode::NodeType() {
  return BoundNodeType::AssignmentExpression;
}

Type BoundAssignmentExpressionNode::GetType() {
  return mBoundExpression->GetType();
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

BoundAssignmentOperatorType BoundAssignmentExpressionNode::OperatorType() {
  return mAssignmentOperator->AssignmentType();
}

BoundAssignmentExpressionNode::BoundAssignmentExpressionNode(
    std::shared_ptr<VariableSymbol> variable,
    std::unique_ptr<BoundExpressionNode> boundExpression,
    std::shared_ptr<BoundAssignmentOperator> assignmentOperator)
    : BoundExpressionNode(), mVariable(variable),
      mBoundExpression(std::move(boundExpression)),
      mAssignmentOperator(assignmentOperator) {}