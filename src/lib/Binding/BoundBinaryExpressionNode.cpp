// Copyright (c) 2022 F. Lotfi All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "BoundBinaryExpressionNode.h"

const std::shared_ptr<BoundBinaryOperator> BoundBinaryOperator::sOperators[] = {
    std::make_shared<BoundBinaryOperator>(SyntaxKind::UnknownToken,
                                          BoundBinaryOperatorKind::Addition,
                                          Type::Unknown),
    std::make_shared<BoundBinaryOperator>(
        SyntaxKind::PlusToken, BoundBinaryOperatorKind::Addition, Type::Number),
    std::make_shared<BoundBinaryOperator>(SyntaxKind::MinusToken,
                                          BoundBinaryOperatorKind::Subtraction,
                                          Type::Number),
    std::make_shared<BoundBinaryOperator>(
        SyntaxKind::StarToken, BoundBinaryOperatorKind::Multiplication,
        Type::Number),
    std::make_shared<BoundBinaryOperator>(SyntaxKind::SlashToken,
                                          BoundBinaryOperatorKind::Division,
                                          Type::Number),
    std::make_shared<BoundBinaryOperator>(
        SyntaxKind::EqualsEqualsToken, BoundBinaryOperatorKind::Equals,
        Type::Number, Type::Number, Type::Boolean),
    std::make_shared<BoundBinaryOperator>(
        SyntaxKind::EqualsEqualsToken, BoundBinaryOperatorKind::Equals,
        Type::Boolean, Type::Boolean, Type::Boolean),
    std::make_shared<BoundBinaryOperator>(
        SyntaxKind::BangEqualsToken, BoundBinaryOperatorKind::NotEquals,
        Type::Number, Type::Number, Type::Boolean),
    std::make_shared<BoundBinaryOperator>(
        SyntaxKind::BangEqualsToken, BoundBinaryOperatorKind::NotEquals,
        Type::Boolean, Type::Boolean, Type::Boolean),
    std::make_shared<BoundBinaryOperator>(SyntaxKind::AmpersandAmpersandToken,
                                          BoundBinaryOperatorKind::LogicalAnd,
                                          Type::Boolean),
    std::make_shared<BoundBinaryOperator>(SyntaxKind::PipePipeToken,
                                          BoundBinaryOperatorKind::LogicalOr,
                                          Type::Boolean),

    std::make_shared<BoundBinaryOperator>(SyntaxKind::AmpersandToken,
                                          BoundBinaryOperatorKind::BitwiseAnd,
                                          Type::Number),
    std::make_shared<BoundBinaryOperator>(SyntaxKind::PipeToken,
                                          BoundBinaryOperatorKind::BitwiseOr,
                                          Type::Number),
    std::make_shared<BoundBinaryOperator>(SyntaxKind::HatToken,
                                          BoundBinaryOperatorKind::BitwiseXor,
                                          Type::Number),

    std::make_shared<BoundBinaryOperator>(SyntaxKind::GreaterToken,
                                          BoundBinaryOperatorKind::GreaterThan,
                                          Type::Number),
    std::make_shared<BoundBinaryOperator>(
        SyntaxKind::GreaterOrEqualsToken,
        BoundBinaryOperatorKind::GreaterThanOrEqualTo, Type::Number),
    std::make_shared<BoundBinaryOperator>(
        SyntaxKind::LessToken, BoundBinaryOperatorKind::LessThan, Type::Number),
    std::make_shared<BoundBinaryOperator>(
        SyntaxKind::LessOrEqualsToken,
        BoundBinaryOperatorKind::LessThanOrEqualTo, Type::Number),
};

BoundBinaryOperator::BoundBinaryOperator(SyntaxKind syntaxKind,
                                         BoundBinaryOperatorKind boundKind,
                                         Type type)
    : mSyntaxKind(syntaxKind), mBoundKind(boundKind), mLeftOperandType(type),
      mRightOperandType(type), mEvalType(type) {}

BoundBinaryOperator::BoundBinaryOperator(SyntaxKind syntaxKind,
                                         BoundBinaryOperatorKind boundKind,
                                         Type leftOperandType,
                                         Type rightOperandType, Type evalType)
    : mSyntaxKind(syntaxKind), mBoundKind(boundKind),
      mLeftOperandType(leftOperandType), mRightOperandType(rightOperandType),
      mEvalType(evalType) {}

const std::shared_ptr<BoundBinaryOperator>
BoundBinaryOperator::GetBindFailure() {
  return sOperators[0];
}

const std::shared_ptr<BoundBinaryOperator>
BoundBinaryOperator::Bind(SyntaxKind syntaxKind, Type leftOperandType,
                          Type rightOperandType) {
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
Type BoundBinaryOperator::LeftOperandType() { return mLeftOperandType; }

Type BoundBinaryOperator::RightOperandType() { return mRightOperandType; }

Type BoundBinaryOperator::GetType() { return mEvalType; }

BoundBinaryExpressionNode::BoundBinaryExpressionNode(
    std::unique_ptr<BoundExpressionNode> left,
    const std::shared_ptr<BoundBinaryOperator> op,
    std::unique_ptr<BoundExpressionNode> right)
    : BoundExpressionNode(), mLeft(std::move(left)), mOperator(op),
      mRight(std::move(right)) {}

BoundNodeKind BoundBinaryExpressionNode::NodeKind() {
  return BoundNodeKind::BinaryExpression;
}

Type BoundBinaryExpressionNode::GetType() { return mOperator->GetType(); }

BoundExpressionNode *BoundBinaryExpressionNode::Left() { return mLeft.get(); }

BoundExpressionNode *BoundBinaryExpressionNode::Right() { return mRight.get(); }

BoundBinaryOperatorKind BoundBinaryExpressionNode::OperatorKind() {
  return mOperator->BoundKind();
}