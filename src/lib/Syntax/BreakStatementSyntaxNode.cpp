// Copyright (c) 2026 F. Lotfi All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "BreakStatementSyntaxNode.h"

BreakStatementSyntaxNode::BreakStatementSyntaxNode(
    std::shared_ptr<SyntaxToken> breakKeyword)
    : StatementSyntaxNode(SyntaxKind::BreakStatement),
      mBreakKeyword(std::move(breakKeyword)) {
  mVecExpressionNodes = {mBreakKeyword.get()};
}

const std::shared_ptr<SyntaxToken> &
BreakStatementSyntaxNode::BreakKeyword() const {
  return mBreakKeyword;
}
