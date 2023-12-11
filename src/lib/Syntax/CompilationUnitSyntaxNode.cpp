/*
 * Copyright (c) 2022 F. Lotfi All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#include "CompilationUnitSyntaxNode.h"

CompilationUnitSyntaxNode::CompilationUnitSyntaxNode(
    std::unique_ptr<StatementSyntaxNode> statement,
    std::shared_ptr<SyntaxToken> endOfFileToken)
    : StatementSyntaxNode(SyntaxKind::CompilationUnit),
      mStatement(std::move(statement)), mEndOfFileToken(endOfFileToken) {
        mVecExpressionNodes.push_back(mStatement.get());
        mVecExpressionNodes.push_back(mEndOfFileToken.get());
      }

const StatementSyntaxNode *CompilationUnitSyntaxNode::Statement() const {
  return mStatement.get();
}
std::shared_ptr<SyntaxToken> CompilationUnitSyntaxNode::EndOfFileToken() {
  return mEndOfFileToken;
}
