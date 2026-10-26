// Copyright (c) 2026 F. Lotfi All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "ReturnStatementSyntaxNode.h"

ReturnStatementSyntaxNode::ReturnStatementSyntaxNode(
    std::shared_ptr<SyntaxToken> returnKeyword,
    std::unique_ptr<ExpressionNode> expression)
    : StatementSyntaxNode(SyntaxKind::ReturnStatement),
      mReturnKeyword(returnKeyword), mExpression(std::move(expression)) {
  mVecExpressionNodes.push_back(mReturnKeyword.get());
  if (mExpression) {
    mVecExpressionNodes.push_back(mExpression.get());
  }
}

std::shared_ptr<SyntaxToken> ReturnStatementSyntaxNode::ReturnKeyword() const {
  return mReturnKeyword;
}

const ExpressionNode *ReturnStatementSyntaxNode::Expression() const {
  return mExpression.get();
}
