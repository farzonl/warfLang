/*
 * Copyright (c) 2022 F. Lotfi All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#pragma once
#include "BoundExpressionNode.h"
#include "Syntax/SyntaxType.h"
#include <memory>

enum class BoundBinaryOperatorType {
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

static const std::unordered_map<BoundBinaryOperatorType, std::string>
    BoundBinaryTypeStrMap = {
        {BoundBinaryOperatorType::Addition, "Addition"},
        {BoundBinaryOperatorType::Subtraction, "Subtraction"},
        {BoundBinaryOperatorType::Multiplication, "Multiplication"},
        {BoundBinaryOperatorType::Division, "Division"},
        {BoundBinaryOperatorType::Equals, "Equals"},
        {BoundBinaryOperatorType::NotEquals, "NotEquals"},
        {BoundBinaryOperatorType::LogicalAnd, "LogicalAnd"},
        {BoundBinaryOperatorType::LogicalOr, "LogicalOr"},
        {BoundBinaryOperatorType::BitwiseAnd, "BitwiseAnd"},
        {BoundBinaryOperatorType::BitwiseOr, "BitwiseOr"},
        {BoundBinaryOperatorType::BitwiseXor, "BitwiseXor"},
        {BoundBinaryOperatorType::BitwiseOr, "BitwiseOr,"},
        {BoundBinaryOperatorType::BitwiseXor, "BitwiseXor,"},
        {BoundBinaryOperatorType::GreaterThan, "GreaterThan,"},
        {BoundBinaryOperatorType::LessThan, "LessThan,"},
        {BoundBinaryOperatorType::LessThanOrEqualTo, "LessThanOrEqualTo,"},
        {BoundBinaryOperatorType::GreaterThanOrEqualTo, "GreaterThanOrEqualTo"},
};

class BoundBinaryOperator {
public:
  static const std::shared_ptr<BoundBinaryOperator>
  Bind(SyntaxType syntaxType, Type leftOperandType, Type rightOperandType);
  SyntaxType GetSyntaxType();
  BoundBinaryOperatorType BoundType();
  Type LeftOperandType();  // expected Type
  Type RightOperandType(); // expected Type
  Type GetType();          // resulting Type
  static const std::shared_ptr<BoundBinaryOperator> GetBindFailure();
  // TODO see if there is a way to make shared_ptr a friend function
  BoundBinaryOperator(SyntaxType syntaxType, BoundBinaryOperatorType boundType,
                      Type type);
  BoundBinaryOperator(SyntaxType syntaxType, BoundBinaryOperatorType boundType,
                      Type leftOperandType, Type rightOperandType,
                      Type evalType);

private:
  SyntaxType mSyntaxType;
  BoundBinaryOperatorType mBoundType;
  Type mLeftOperandType;
  Type mRightOperandType;
  Type mEvalType;
  static const std::shared_ptr<BoundBinaryOperator> sOperators[];
  BoundBinaryOperator() = delete;
  friend class BoundBinaryExpressionNode;
};

class BoundBinaryExpressionNode : public BoundExpressionNode {
public:
  BoundBinaryExpressionNode(std::unique_ptr<BoundExpressionNode> left,
                            const std::shared_ptr<BoundBinaryOperator> op,
                            std::unique_ptr<BoundExpressionNode> right);
  virtual BoundNodeType NodeType() override;
  virtual Type GetType() override;
  BoundExpressionNode *Left();
  BoundExpressionNode *Right();
  BoundBinaryOperatorType OperatorType();

private:
  std::unique_ptr<BoundExpressionNode> mLeft;
  std::unique_ptr<BoundExpressionNode> mRight;
  const std::shared_ptr<BoundBinaryOperator> mOperator;
};