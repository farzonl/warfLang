// Copyright (c) 2026 F. Lotfi All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "ContinueStatementSyntaxNode.h"

ContinueStatementSyntaxNode::ContinueStatementSyntaxNode(
    std::shared_ptr<SyntaxToken> continueKeyword)
    : StatementSyntaxNode(SyntaxKind::ContinueStatement),
      mContinueKeyword(std::move(continueKeyword)) {
  mVecExpressionNodes = {mContinueKeyword.get()};
}

const std::shared_ptr<SyntaxToken> &
ContinueStatementSyntaxNode::ContinueKeyword() const {
  return mContinueKeyword;
}
