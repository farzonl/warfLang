// Copyright (c) 2022 F. Lotfi All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "AssignmentExpressionNode.h"

AssignmentExpressionNode::AssignmentExpressionNode(
    std::shared_ptr<SyntaxToken> identifierToken,
    std::shared_ptr<SyntaxToken> equalsToken,
    std::unique_ptr<ExpressionNode> expression)
    : ExpressionNode(SyntaxKind::AssignmentExpression),
      mIdentifier(identifierToken), mAssignmentOperator(equalsToken),
      mExpression(std::move(expression)) {
  mVecExpressionNodes.push_back(identifierToken.get());
  mVecExpressionNodes.push_back(mAssignmentOperator.get());
  mVecExpressionNodes.push_back(mExpression.get());
}

std::shared_ptr<SyntaxToken> AssignmentExpressionNode::IdentifierToken() {
  return mIdentifier;
}

std::shared_ptr<SyntaxToken> AssignmentExpressionNode::AssignmentToken() {
  return mAssignmentOperator;
}

ExpressionNode *AssignmentExpressionNode::Expression() {
  return mExpression.get();
}