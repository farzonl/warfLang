// Copyright (c) 2022 F. Lotfi All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "UnaryExpressionNode.h"

UnaryExpressionNode::UnaryExpressionNode(
    std::shared_ptr<SyntaxToken> operatorToken,
    std::unique_ptr<ExpressionNode> operand)
    : ExpressionNode(SyntaxKind::UnaryExpression), mOperator(operatorToken),
      mOperand(std::move(operand)) {
  mVecExpressionNodes.push_back(mOperator.get());
  mVecExpressionNodes.push_back(mOperand.get());
}

ExpressionNode *UnaryExpressionNode::Operand() { return mOperand.get(); }
std::shared_ptr<SyntaxToken> UnaryExpressionNode::Operator() {
  return mOperator;
}
