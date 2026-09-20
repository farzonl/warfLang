// Copyright (c) 2026 F. Lotfi All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "IfStatementSyntaxNode.h"

IfStatementSyntaxNode::IfStatementSyntaxNode(
    std::shared_ptr<SyntaxToken> ifKeyword,
    std::shared_ptr<SyntaxToken> openParenthesisToken,
    std::unique_ptr<ExpressionNode> condition,
    std::shared_ptr<SyntaxToken> closeParenthesisToken,
    std::unique_ptr<StatementSyntaxNode> thenStatement,
    std::shared_ptr<SyntaxToken> elseKeyword,
    std::unique_ptr<StatementSyntaxNode> elseStatement)
    : StatementSyntaxNode(SyntaxKind::IfStatement),
      mIfKeyword(std::move(ifKeyword)),
      mOpenParenthesisToken(std::move(openParenthesisToken)),
      mCloseParenthesisToken(std::move(closeParenthesisToken)),
      mCondition(std::move(condition)),
      mThenStatement(std::move(thenStatement)),
      mElseKeyword(std::move(elseKeyword)),
      mElseStatement(std::move(elseStatement)) {
  mVecExpressionNodes = {mIfKeyword.get(), mOpenParenthesisToken.get(),
                         mCondition.get(), mCloseParenthesisToken.get(),
                         mThenStatement.get()};
  if (mElseKeyword) {
    mVecExpressionNodes.push_back(mElseKeyword.get());
    mVecExpressionNodes.push_back(mElseStatement.get());
  }
}

std::unique_ptr<ExpressionNode> &IfStatementSyntaxNode::Condition() {
  return mCondition;
}

const StatementSyntaxNode *IfStatementSyntaxNode::ThenStatement() const {
  return mThenStatement.get();
}

const StatementSyntaxNode *IfStatementSyntaxNode::ElseStatement() const {
  return mElseStatement.get();
}
