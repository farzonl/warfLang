/*
 * Copyright (c) 2022 F. Lotfi All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#include "CompilationUnitSyntaxNode.h"

CompilationUnitSyntaxNode::CompilationUnitSyntaxNode(
    std::vector<std::unique_ptr<StatementSyntaxNode>> statements,
    std::shared_ptr<SyntaxToken> endOfFileToken)
    : StatementSyntaxNode(SyntaxKind::CompilationUnit),
      mStatements(std::move(statements)), mEndOfFileToken(endOfFileToken) {
  for (auto &statement : mStatements) {
    mVecExpressionNodes.push_back(statement.get());
  }
  mVecExpressionNodes.push_back(mEndOfFileToken.get());
}

CompilationUnitSyntaxNode::CompilationUnitSyntaxNode(
    std::unique_ptr<StatementSyntaxNode> statement,
    std::shared_ptr<SyntaxToken> endOfFileToken)
    : CompilationUnitSyntaxNode(
          std::vector<std::unique_ptr<StatementSyntaxNode>>(),
          std::move(endOfFileToken)) {
  mStatements = TemplateList<std::unique_ptr<StatementSyntaxNode>>();
  std::vector<std::unique_ptr<StatementSyntaxNode>> statements;
  statements.push_back(std::move(statement));
  mStatements = TemplateList<std::unique_ptr<StatementSyntaxNode>>(
      std::move(statements));
  mVecExpressionNodes.insert(mVecExpressionNodes.begin(),
                             mStatements[0].get());
}

const StatementSyntaxNode *CompilationUnitSyntaxNode::Statement() const {
  return mStatements.empty() ? nullptr : mStatements[0].get();
}

const TemplateList<std::unique_ptr<StatementSyntaxNode>> &
CompilationUnitSyntaxNode::Statements() const {
  return mStatements;
}
std::shared_ptr<SyntaxToken> CompilationUnitSyntaxNode::EndOfFileToken() {
  return mEndOfFileToken;
}
