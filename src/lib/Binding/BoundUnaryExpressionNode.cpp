// Copyright (c) 2022 F. Lotfi All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "BoundUnaryExpressionNode.h"

const std::shared_ptr<BoundUnaryOperator> BoundUnaryOperator::sOperators[] = {
    std::make_shared<BoundUnaryOperator>(SyntaxKind::UnknownToken,
                                         BoundUnaryOperatorKind::Identity,
                                         Value::Type::Unknown),
    std::make_shared<BoundUnaryOperator>(
        SyntaxKind::BangToken, BoundUnaryOperatorKind::LogicalNegation,
        Value::Type::Boolean),
    std::make_shared<BoundUnaryOperator>(SyntaxKind::MinusToken,
                                         BoundUnaryOperatorKind::Negation,
                                         Value::Type::Number),
    std::make_shared<BoundUnaryOperator>(SyntaxKind::PlusToken,
                                         BoundUnaryOperatorKind::Identity,
                                         Value::Type::Number),
    std::make_shared<BoundUnaryOperator>(SyntaxKind::TildeToken,
                                         BoundUnaryOperatorKind::BitwiseNot,
                                         Value::Type::Number),
};

BoundUnaryOperator::BoundUnaryOperator(SyntaxKind syntaxKind,
                                       BoundUnaryOperatorKind unaryType,
                                       Value::Type operandValueType)
    : mSyntaxKind(syntaxKind), mUnaryKind(unaryType),
      mOperandType(operandValueType) {}

const std::shared_ptr<BoundUnaryOperator> BoundUnaryOperator::GetBindFailure() {
  return sOperators[0];
}

const std::shared_ptr<BoundUnaryOperator>
BoundUnaryOperator::Bind(SyntaxKind syntaxKind, Value::Type operandType) {
  for (std::shared_ptr<BoundUnaryOperator> op :
       BoundUnaryOperator::sOperators) {
    if (op->GetSyntaxKind() == syntaxKind && op->OperandType() == operandType) {
      return op;
    }
  }
  return GetBindFailure();
}

SyntaxKind BoundUnaryOperator::GetSyntaxKind() { return mSyntaxKind; }

Value::Type BoundUnaryOperator::OperandType() { return mOperandType; }

Value::Type BoundUnaryOperator::EvalType() { return mEvalType; }

BoundUnaryOperatorKind BoundUnaryOperator::UnaryKind() { return mUnaryKind; }

BoundUnaryExpressionNode::BoundUnaryExpressionNode(
    const std::shared_ptr<BoundUnaryOperator> op,
    std::unique_ptr<BoundExpressionNode> operand)
    : BoundExpressionNode(), mOperator(op), mOperand(std::move(operand)) {}

BoundNodeKind BoundUnaryExpressionNode::Kind() {
  return BoundNodeKind::UnaryExpression;
}

BoundExpressionNode *BoundUnaryExpressionNode::Operand() {
  return mOperand.get();
}

Value::Type BoundUnaryExpressionNode::Type() { return mOperand->Type(); }

BoundUnaryOperatorKind BoundUnaryExpressionNode::OperatorKind() {
  return mOperator->UnaryKind();
}
