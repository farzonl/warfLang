// Copyright (c) 2022 F. Lotfi All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "BlockStatementSyntaxNode.h"

BlockStatementSyntaxNode::BlockStatementSyntaxNode(std::shared_ptr<SyntaxToken> openBraceToken,
   std::vector<std::unique_ptr<StatementSyntaxNode>> statements,
   std::shared_ptr<SyntaxToken> closeBraceToken) : StatementSyntaxNode(SyntaxKind::BlockStatement), 
   mOpenBraceToken(openBraceToken), mStatements(std::move(statements)), mCloseBraceToken(closeBraceToken) {}


std::shared_ptr<SyntaxToken> BlockStatementSyntaxNode::OpenBraceToken() {
    return mOpenBraceToken;
}
  const TemplateList<std::unique_ptr<StatementSyntaxNode>>& BlockStatementSyntaxNode::Statements() const {
    return mStatements;
  }
  std::shared_ptr<SyntaxToken> BlockStatementSyntaxNode::CloseBraceToken() {
    return mCloseBraceToken;
  }