/*
 * Copyright (c) 2022 F. Lotfi All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#pragma once

#include <memory>

#include "BoundExpressionNode.h"
#include "Syntax/SyntaxType.h"
#include "ValueType.h"

enum class BoundUnaryOperatorType {
  Identity,
  Negation,
  LogicalNegation,
  BitwiseNot
};

static const std::unordered_map<BoundUnaryOperatorType, std::string>
    BoundUnaryTypeStrMap = {
        {BoundUnaryOperatorType::Identity, "Identity"},
        {BoundUnaryOperatorType::Negation, "Negation"},
        {BoundUnaryOperatorType::LogicalNegation, "LogicalNegation"},
        {BoundUnaryOperatorType::BitwiseNot, "BitwiseNot"},
};

class BoundUnaryOperator {
public:
  static const std::shared_ptr<BoundUnaryOperator> Bind(SyntaxType syntaxType,
                                                        Type operandType);
  SyntaxType GetSyntaxType();
  BoundUnaryOperatorType UnaryType();
  Type OperandType(); // expected Type
  Type EvalType();    // resulting Type
  static const std::shared_ptr<BoundUnaryOperator> GetBindFailure();
  BoundUnaryOperator(SyntaxType syntaxType, BoundUnaryOperatorType unaryType,
                     Type operandValueType);

private:
  SyntaxType mSyntaxType;
  BoundUnaryOperatorType mUnaryType;
  Type mOperandType;
  Type mEvalType;

  static const std::shared_ptr<BoundUnaryOperator> sOperators[];
  BoundUnaryOperator() = delete;
  friend class BoundUnaryExpressionNode;
};

class BoundUnaryExpressionNode : public BoundExpressionNode {
public:
  BoundUnaryExpressionNode(const std::shared_ptr<BoundUnaryOperator> op,
                           std::unique_ptr<BoundExpressionNode> operand);
  virtual ~BoundUnaryExpressionNode() {}
  virtual BoundNodeType NodeType() override;
  virtual Type GetType() override;
  BoundExpressionNode *Operand();
  BoundUnaryOperatorType OperatorType();

private:
  const std::shared_ptr<BoundUnaryOperator> mOperator;
  std::unique_ptr<BoundExpressionNode> mOperand;
};