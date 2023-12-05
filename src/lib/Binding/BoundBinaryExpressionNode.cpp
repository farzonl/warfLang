// Copyright (c) 2022 F. Lotfi All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "BoundBinaryExpressionNode.h"

const std::shared_ptr<BoundBinaryOperator> BoundBinaryOperator::sOperators[] = {
    std::make_shared<BoundBinaryOperator>(SyntaxKind::UnknownToken,
                                          BoundBinaryOperatorKind::Addition,
                                          Value::Type::Unknown),
    std::make_shared<BoundBinaryOperator>(
        SyntaxKind::PlusToken, BoundBinaryOperatorKind::Addition, Value::Type::Number),
    std::make_shared<BoundBinaryOperator>(SyntaxKind::MinusToken,
                                          BoundBinaryOperatorKind::Subtraction,
                                          Value::Type::Number),
    std::make_shared<BoundBinaryOperator>(
        SyntaxKind::StarToken, BoundBinaryOperatorKind::Multiplication,
        Value::Type::Number),
    std::make_shared<BoundBinaryOperator>(SyntaxKind::SlashToken,
                                          BoundBinaryOperatorKind::Division,
                                          Value::Type::Number),
    std::make_shared<BoundBinaryOperator>(
        SyntaxKind::EqualsEqualsToken, BoundBinaryOperatorKind::Equals,
        Value::Type::Number, Value::Type::Number, Value::Type::Boolean),
    std::make_shared<BoundBinaryOperator>(
        SyntaxKind::EqualsEqualsToken, BoundBinaryOperatorKind::Equals,
        Value::Type::Boolean, Value::Type::Boolean, Value::Type::Boolean),
    std::make_shared<BoundBinaryOperator>(
        SyntaxKind::BangEqualsToken, BoundBinaryOperatorKind::NotEquals,
        Value::Type::Number, Value::Type::Number, Value::Type::Boolean),
    std::make_shared<BoundBinaryOperator>(
        SyntaxKind::BangEqualsToken, BoundBinaryOperatorKind::NotEquals,
        Value::Type::Boolean, Value::Type::Boolean, Value::Type::Boolean),
    std::make_shared<BoundBinaryOperator>(SyntaxKind::AmpersandAmpersandToken,
                                          BoundBinaryOperatorKind::LogicalAnd,
                                          Value::Type::Boolean),
    std::make_shared<BoundBinaryOperator>(SyntaxKind::PipePipeToken,
                                          BoundBinaryOperatorKind::LogicalOr,
                                          Value::Type::Boolean),

    std::make_shared<BoundBinaryOperator>(SyntaxKind::AmpersandToken,
                                          BoundBinaryOperatorKind::BitwiseAnd,
                                          Value::Type::Number),
    std::make_shared<BoundBinaryOperator>(SyntaxKind::PipeToken,
                                          BoundBinaryOperatorKind::BitwiseOr,
                                          Value::Type::Number),
    std::make_shared<BoundBinaryOperator>(SyntaxKind::HatToken,
                                          BoundBinaryOperatorKind::BitwiseXor,
                                          Value::Type::Number),

    std::make_shared<BoundBinaryOperator>(SyntaxKind::GreaterToken,
                                          BoundBinaryOperatorKind::GreaterThan,
                                          Value::Type::Number),
    std::make_shared<BoundBinaryOperator>(
        SyntaxKind::GreaterOrEqualsToken,
        BoundBinaryOperatorKind::GreaterThanOrEqualTo, Value::Type::Number),
    std::make_shared<BoundBinaryOperator>(
        SyntaxKind::LessToken, BoundBinaryOperatorKind::LessThan, Value::Type::Number),
    std::make_shared<BoundBinaryOperator>(
        SyntaxKind::LessOrEqualsToken,
        BoundBinaryOperatorKind::LessThanOrEqualTo, Value::Type::Number),
};

BoundBinaryOperator::BoundBinaryOperator(SyntaxKind syntaxKind,
                                         BoundBinaryOperatorKind boundKind,
                                         Value::Type type)
    : mSyntaxKind(syntaxKind), mBoundKind(boundKind), mLeftOperandType(type),
      mRightOperandType(type), mEvalType(type) {}

BoundBinaryOperator::BoundBinaryOperator(SyntaxKind syntaxKind,
                                         BoundBinaryOperatorKind boundKind,
                                         Value::Type leftOperandType,
                                         Value::Type rightOperandType, Value::Type evalType)
    : mSyntaxKind(syntaxKind), mBoundKind(boundKind),
      mLeftOperandType(leftOperandType), mRightOperandType(rightOperandType),
      mEvalType(evalType) {}

const std::shared_ptr<BoundBinaryOperator>
BoundBinaryOperator::GetBindFailure() {
  return sOperators[0];
}

const std::shared_ptr<BoundBinaryOperator>
BoundBinaryOperator::Bind(SyntaxKind syntaxKind, Value::Type leftOperandType,
                          Value::Type rightOperandType) {
  for (std::shared_ptr<BoundBinaryOperator> op :
       BoundBinaryOperator::sOperators) {
    if (op->GetSyntaxKind() == syntaxKind &&
        op->LeftOperandType() == leftOperandType &&
        op->RightOperandType() == rightOperandType) {
      return op;
    }
  }
  return GetBindFailure();
}

SyntaxKind BoundBinaryOperator::GetSyntaxKind() { return mSyntaxKind; }

BoundBinaryOperatorKind BoundBinaryOperator::BoundKind() { return mBoundKind; }
Value::Type BoundBinaryOperator::LeftOperandType() { return mLeftOperandType; }

Value::Type BoundBinaryOperator::RightOperandType() { return mRightOperandType; }

Value::Type BoundBinaryOperator::Type() { return mEvalType; }

BoundBinaryExpressionNode::BoundBinaryExpressionNode(
    std::unique_ptr<BoundExpressionNode> left,
    const std::shared_ptr<BoundBinaryOperator> op,
    std::unique_ptr<BoundExpressionNode> right)
    : BoundExpressionNode(), mLeft(std::move(left)), mOperator(op),
      mRight(std::move(right)) {}

BoundNodeKind BoundBinaryExpressionNode::NodeKind() {
  return BoundNodeKind::BinaryExpression;
}

Value::Type BoundBinaryExpressionNode::Type() { return mOperator->Type(); }

BoundExpressionNode *BoundBinaryExpressionNode::Left() { return mLeft.get(); }

BoundExpressionNode *BoundBinaryExpressionNode::Right() { return mRight.get(); }

BoundBinaryOperatorKind BoundBinaryExpressionNode::OperatorKind() {
  return mOperator->BoundKind();
}