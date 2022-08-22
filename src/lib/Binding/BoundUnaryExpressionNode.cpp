// Copyright (c) 2022 F. Lotfi All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "BoundUnaryExpressionNode.h"

const BoundUnaryOperator BoundUnaryOperator::sOperators[] = {
    BoundUnaryOperator(SyntaxType::UnknownToken,
                       BoundUnaryOperatorType::Identity, Type::Unknown),
    BoundUnaryOperator(SyntaxType::BangToken,
                       BoundUnaryOperatorType::LogicalNegation, Type::Boolean),
    BoundUnaryOperator(SyntaxType::MinusToken, BoundUnaryOperatorType::Negation,
                       Type::Number),
    BoundUnaryOperator(SyntaxType::PlusToken, BoundUnaryOperatorType::Identity,
                       Type::Number),
};

BoundUnaryOperator::BoundUnaryOperator(SyntaxType syntaxType,
                                       BoundUnaryOperatorType unaryType,
                                       Type operandValueType)
    : mSyntaxType(syntaxType), mUnaryType(unaryType),
      mOperandType(operandValueType), mValueType(operandValueType) {}

const BoundUnaryOperator &BoundUnaryOperator::GetBindFailure() {
  return sOperators[0];
}

const BoundUnaryOperator &BoundUnaryOperator::Bind(SyntaxType syntaxType,
                                                   Type operandType) {
  for (auto op : sOperators) {
    if (op.GetSyntaxType() == syntaxType && op.OperandType() == operandType) {
      return op;
    }
  }
  return GetBindFailure();
}

BoundUnaryExpressionNode::BoundUnaryExpressionNode(
    BoundUnaryOperator &op, std::unique_ptr<BoundExpressionNode> operand)
    : BoundExpressionNode() {}
