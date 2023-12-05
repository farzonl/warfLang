// Copyright (c) 2022 F. Lotfi All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "BoundUnaryExpressionNode.h"

const std::shared_ptr<BoundUnaryOperator> BoundUnaryOperator::sOperators[] = {
    std::make_shared<BoundUnaryOperator>(SyntaxKind::UnknownToken,
                                         BoundUnaryOperatorKind::Identity,
                                         Type::Unknown),
    std::make_shared<BoundUnaryOperator>(
        SyntaxKind::BangToken, BoundUnaryOperatorKind::LogicalNegation,
        Type::Boolean),
    std::make_shared<BoundUnaryOperator>(
        SyntaxKind::MinusToken, BoundUnaryOperatorKind::Negation, Type::Number),
    std::make_shared<BoundUnaryOperator>(
        SyntaxKind::PlusToken, BoundUnaryOperatorKind::Identity, Type::Number),
    std::make_shared<BoundUnaryOperator>(SyntaxKind::TildeToken,
                                         BoundUnaryOperatorKind::BitwiseNot,
                                         Type::Number),
};

BoundUnaryOperator::BoundUnaryOperator(SyntaxKind syntaxKind,
                                       BoundUnaryOperatorKind unaryType,
                                       Type operandValueType)
    : mSyntaxKind(syntaxKind), mUnaryKind(unaryType),
      mOperandType(operandValueType) {}

const std::shared_ptr<BoundUnaryOperator> BoundUnaryOperator::GetBindFailure() {
  return sOperators[0];
}

const std::shared_ptr<BoundUnaryOperator>
BoundUnaryOperator::Bind(SyntaxKind syntaxKind, Type operandType) {
  for (std::shared_ptr<BoundUnaryOperator> op :
       BoundUnaryOperator::sOperators) {
    if (op->GetSyntaxKind() == syntaxKind && op->OperandType() == operandType) {
      return op;
    }
  }
  return GetBindFailure();
}

SyntaxKind BoundUnaryOperator::GetSyntaxKind() { return mSyntaxKind; }

Type BoundUnaryOperator::OperandType() { return mOperandType; }

Type BoundUnaryOperator::EvalType() { return mEvalType; }

BoundUnaryOperatorKind BoundUnaryOperator::UnaryKind() { return mUnaryKind; }

BoundUnaryExpressionNode::BoundUnaryExpressionNode(
    const std::shared_ptr<BoundUnaryOperator> op,
    std::unique_ptr<BoundExpressionNode> operand)
    : BoundExpressionNode(), mOperator(op), mOperand(std::move(operand)) {}

BoundNodeKind BoundUnaryExpressionNode::NodeKind() {
  return BoundNodeKind::UnaryExpression;
}

BoundExpressionNode *BoundUnaryExpressionNode::Operand() {
  return mOperand.get();
}

Type BoundUnaryExpressionNode::GetType() { return mOperand->GetType(); }

BoundUnaryOperatorKind BoundUnaryExpressionNode::OperatorKind() {
  return mOperator->UnaryKind();
}
