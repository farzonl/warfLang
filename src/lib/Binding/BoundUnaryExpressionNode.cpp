// Copyright (c) 2022 F. Lotfi All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "BoundUnaryExpressionNode.h"

const std::shared_ptr<BoundUnaryOperator> BoundUnaryOperator::sOperators[] = {
    std::make_shared<BoundUnaryOperator>(SyntaxType::UnknownToken,
                                         BoundUnaryOperatorType::Identity,
                                         Type::Unknown),
    std::make_shared<BoundUnaryOperator>(
        SyntaxType::BangToken, BoundUnaryOperatorType::LogicalNegation,
        Type::Boolean),
    std::make_shared<BoundUnaryOperator>(
        SyntaxType::MinusToken, BoundUnaryOperatorType::Negation, Type::Number),
    std::make_shared<BoundUnaryOperator>(
        SyntaxType::PlusToken, BoundUnaryOperatorType::Identity, Type::Number),
};

BoundUnaryOperator::BoundUnaryOperator(SyntaxType syntaxType,
                                       BoundUnaryOperatorType unaryType,
                                       Type operandValueType)
    : mSyntaxType(syntaxType), mUnaryType(unaryType),
      mOperandType(operandValueType) {}

const std::shared_ptr<BoundUnaryOperator> BoundUnaryOperator::GetBindFailure() {
  return sOperators[0];
}

const std::shared_ptr<BoundUnaryOperator>
BoundUnaryOperator::Bind(SyntaxType syntaxType, Type operandType) {
  for (std::shared_ptr<BoundUnaryOperator> op :
       BoundUnaryOperator::sOperators) {
    if (op->GetSyntaxType() == syntaxType && op->OperandType() == operandType) {
      return op;
    }
  }
  std::cerr << "Unexpected unary operator: " << syntaxType << std::endl;
  return GetBindFailure();
}

SyntaxType BoundUnaryOperator::GetSyntaxType() { return mSyntaxType; }

Type BoundUnaryOperator::OperandType() { return mOperandType; }

Type BoundUnaryOperator::EvalType() { return mEvalType; }

BoundUnaryOperatorType BoundUnaryOperator::UnaryType() { return mUnaryType; }

BoundUnaryExpressionNode::BoundUnaryExpressionNode(
    const std::shared_ptr<BoundUnaryOperator> op,
    std::unique_ptr<BoundExpressionNode> operand)
    : BoundExpressionNode(), mOperator(op), mOperand(std::move(operand)) {}

BoundNodeType BoundUnaryExpressionNode::NodeType() {
  return BoundNodeType::UnaryExpression;
}

BoundExpressionNode *BoundUnaryExpressionNode::Operand() {
  return mOperand.get();
}

Type BoundUnaryExpressionNode::GetType() { return mOperand->GetType(); }

BoundUnaryOperatorType BoundUnaryExpressionNode::OperatorType() {
  return mOperator->UnaryType();
}
