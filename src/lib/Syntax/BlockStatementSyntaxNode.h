/*
 * Copyright (c) 2022 F. Lotfi All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#pragma once
#include "StatementSyntaxNode.h"
#include "SyntaxToken.h"
#include <memory>
#include <vector>
#include "DataStructures/TemplateList.h"

class BlockStatementSyntaxNode  : public StatementSyntaxNode {
public:
  BlockStatementSyntaxNode( std::shared_ptr<SyntaxToken> openBraceToken,
   std::vector<std::unique_ptr<StatementSyntaxNode>> statements,
   std::shared_ptr<SyntaxToken> closeBraceToken);

  virtual ~BlockStatementSyntaxNode () {}

  std::shared_ptr<SyntaxToken> OpenBraceToken();
  const TemplateList<std::unique_ptr<StatementSyntaxNode>>& Statements() const;
  std::shared_ptr<SyntaxToken> CloseBraceToken();
private:
  std::shared_ptr<SyntaxToken> mOpenBraceToken;
  std::shared_ptr<SyntaxToken> mCloseBraceToken;
  TemplateList<std::unique_ptr<StatementSyntaxNode>> mStatements;
};