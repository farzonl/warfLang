/*
 * Copyright (c) 2022 F. Lotfi All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#pragma once

#include <memory>

#include "BoundExpressionNode.h"
#include "Syntax/SyntaxKind.h"
#include "ValueType.h"

enum class BoundUnaryOperatorKind {
  Identity,
  Negation,
  LogicalNegation,
  BitwiseNot
};

static const std::unordered_map<BoundUnaryOperatorKind, std::string>
    BoundUnaryKindStrMap = {
        {BoundUnaryOperatorKind::Identity, "Identity"},
        {BoundUnaryOperatorKind::Negation, "Negation"},
        {BoundUnaryOperatorKind::LogicalNegation, "LogicalNegation"},
        {BoundUnaryOperatorKind::BitwiseNot, "BitwiseNot"},
};

class BoundUnaryOperator {
public:
  static const std::shared_ptr<BoundUnaryOperator>
  Bind(SyntaxKind syntaxKind, Value::Type operandType);
  SyntaxKind GetSyntaxKind();
  BoundUnaryOperatorKind UnaryKind();
  Value::Type OperandType(); // expected Type
  Value::Type EvalType();    // resulting Type
  static const std::shared_ptr<BoundUnaryOperator> GetBindFailure();
  BoundUnaryOperator(SyntaxKind syntaxKind, BoundUnaryOperatorKind unaryType,
                     Value::Type operandValueType);

private:
  SyntaxKind mSyntaxKind;
  BoundUnaryOperatorKind mUnaryKind;
  Value::Type mOperandType;
  Value::Type mEvalType;

  static const std::shared_ptr<BoundUnaryOperator> sOperators[];
  BoundUnaryOperator() = delete;
  friend class BoundUnaryExpressionNode;
};

class BoundUnaryExpressionNode : public BoundExpressionNode {
public:
  BoundUnaryExpressionNode(const std::shared_ptr<BoundUnaryOperator> op,
                           std::unique_ptr<BoundExpressionNode> operand);
  virtual ~BoundUnaryExpressionNode() {}
  virtual BoundNodeKind NodeKind() override;
  virtual Value::Type Type() override;
  BoundExpressionNode *Operand();
  BoundUnaryOperatorKind OperatorKind();

private:
  const std::shared_ptr<BoundUnaryOperator> mOperator;
  std::unique_ptr<BoundExpressionNode> mOperand;
};