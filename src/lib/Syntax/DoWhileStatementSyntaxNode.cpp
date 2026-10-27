// Copyright (c) 2026 F. Lotfi All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "DoWhileStatementSyntaxNode.h"

DoWhileStatementSyntaxNode::DoWhileStatementSyntaxNode(
    std::shared_ptr<SyntaxToken> doKeyword,
    std::unique_ptr<StatementSyntaxNode> body,
    std::shared_ptr<SyntaxToken> whileKeyword,
    std::shared_ptr<SyntaxToken> openParenthesisToken,
    std::unique_ptr<ExpressionNode> condition,
    std::shared_ptr<SyntaxToken> closeParenthesisToken)
    : StatementSyntaxNode(SyntaxKind::DoWhileStatement),
      mDoKeyword(std::move(doKeyword)), mBody(std::move(body)),
      mWhileKeyword(std::move(whileKeyword)),
      mOpenParenthesisToken(std::move(openParenthesisToken)),
      mCondition(std::move(condition)),
      mCloseParenthesisToken(std::move(closeParenthesisToken)) {
  mVecExpressionNodes = {mDoKeyword.get(),    mBody.get(),
                         mWhileKeyword.get(), mOpenParenthesisToken.get(),
                         mCondition.get(),    mCloseParenthesisToken.get()};
}

const StatementSyntaxNode *DoWhileStatementSyntaxNode::Body() const {
  return mBody.get();
}

std::unique_ptr<ExpressionNode> &DoWhileStatementSyntaxNode::Condition() {
  return mCondition;
}
