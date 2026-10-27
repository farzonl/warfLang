// Copyright (c) 2026 F. Lotfi All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "IndexAssignmentExpressionNode.h"

IndexAssignmentExpressionNode::IndexAssignmentExpressionNode(
    std::unique_ptr<IndexExpressionNode> indexExpression,
    std::shared_ptr<SyntaxToken> equalsToken,
    std::unique_ptr<ExpressionNode> value)
    : ExpressionNode(SyntaxKind::IndexAssignmentExpression),
      mIndexExpression(std::move(indexExpression)),
      mEqualsToken(std::move(equalsToken)), mValue(std::move(value)) {
  mVecExpressionNodes.push_back(mIndexExpression.get());
  mVecExpressionNodes.push_back(mEqualsToken.get());
  mVecExpressionNodes.push_back(mValue.get());
}

IndexExpressionNode *IndexAssignmentExpressionNode::IndexExpression() const {
  return mIndexExpression.get();
}

std::shared_ptr<SyntaxToken>
IndexAssignmentExpressionNode::EqualsToken() const {
  return mEqualsToken;
}

ExpressionNode *IndexAssignmentExpressionNode::ValueExpression() const {
  return mValue.get();
}
