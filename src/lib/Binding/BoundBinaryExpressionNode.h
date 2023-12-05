/*
 * Copyright (c) 2022 F. Lotfi All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#pragma once
#include "BoundExpressionNode.h"
#include "Syntax/SyntaxKind.h"
#include <memory>

enum class BoundBinaryOperatorKind {
  Addition,
  Subtraction,
  Multiplication,
  Division,
  Equals,
  NotEquals,
  LogicalAnd,
  LogicalOr,
  BitwiseAnd,
  BitwiseOr,
  BitwiseXor,
  GreaterThan,
  LessThan,
  LessThanOrEqualTo,
  GreaterThanOrEqualTo
};

static const std::unordered_map<BoundBinaryOperatorKind, std::string>
    BoundBinaryKindStrMap = {
        {BoundBinaryOperatorKind::Addition, "Addition"},
        {BoundBinaryOperatorKind::Subtraction, "Subtraction"},
        {BoundBinaryOperatorKind::Multiplication, "Multiplication"},
        {BoundBinaryOperatorKind::Division, "Division"},
        {BoundBinaryOperatorKind::Equals, "Equals"},
        {BoundBinaryOperatorKind::NotEquals, "NotEquals"},
        {BoundBinaryOperatorKind::LogicalAnd, "LogicalAnd"},
        {BoundBinaryOperatorKind::LogicalOr, "LogicalOr"},
        {BoundBinaryOperatorKind::BitwiseAnd, "BitwiseAnd"},
        {BoundBinaryOperatorKind::BitwiseOr, "BitwiseOr"},
        {BoundBinaryOperatorKind::BitwiseXor, "BitwiseXor"},
        {BoundBinaryOperatorKind::BitwiseOr, "BitwiseOr,"},
        {BoundBinaryOperatorKind::BitwiseXor, "BitwiseXor,"},
        {BoundBinaryOperatorKind::GreaterThan, "GreaterThan,"},
        {BoundBinaryOperatorKind::LessThan, "LessThan,"},
        {BoundBinaryOperatorKind::LessThanOrEqualTo, "LessThanOrEqualTo,"},
        {BoundBinaryOperatorKind::GreaterThanOrEqualTo, "GreaterThanOrEqualTo"},
};

class BoundBinaryOperator {
public:
  static const std::shared_ptr<BoundBinaryOperator>
  Bind(SyntaxKind syntaxKind, Value::Type leftOperandType, Value::Type rightOperandType);
  SyntaxKind GetSyntaxKind();
  BoundBinaryOperatorKind BoundKind();
  Value::Type LeftOperandType();  // expected Type
  Value::Type RightOperandType(); // expected Type
  Value::Type Type();      // resulting Type
  static const std::shared_ptr<BoundBinaryOperator> GetBindFailure();
  // TODO see if there is a way to make shared_ptr a friend function
  BoundBinaryOperator(SyntaxKind syntaxKind, BoundBinaryOperatorKind boundKind,
                      Value::Type type);
  BoundBinaryOperator(SyntaxKind syntaxKind, BoundBinaryOperatorKind boundKind,
                      Value::Type leftOperandType, Value::Type rightOperandType,
                      Value::Type evalType);

private:
  SyntaxKind mSyntaxKind;
  BoundBinaryOperatorKind mBoundKind;
  Value::Type mLeftOperandType;
  Value::Type mRightOperandType;
  Value::Type mEvalType;
  static const std::shared_ptr<BoundBinaryOperator> sOperators[];
  BoundBinaryOperator() = delete;
  friend class BoundBinaryExpressionNode;
};

class BoundBinaryExpressionNode : public BoundExpressionNode {
public:
  BoundBinaryExpressionNode(std::unique_ptr<BoundExpressionNode> left,
                            const std::shared_ptr<BoundBinaryOperator> op,
                            std::unique_ptr<BoundExpressionNode> right);
  virtual ~BoundBinaryExpressionNode() {}
  virtual BoundNodeKind NodeKind() override;
  virtual Value::Type Type() override;
  BoundExpressionNode *Left();
  BoundExpressionNode *Right();
  BoundBinaryOperatorKind OperatorKind();

private:
  std::unique_ptr<BoundExpressionNode> mLeft;
  std::unique_ptr<BoundExpressionNode> mRight;
  const std::shared_ptr<BoundBinaryOperator> mOperator;
};