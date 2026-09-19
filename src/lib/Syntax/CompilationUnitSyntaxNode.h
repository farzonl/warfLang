/*
 * Copyright (c) 2022 F. Lotfi All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#pragma once
#include "DataStructures/TemplateList.h"
#include "StatementSyntaxNode.h"
#include "SyntaxToken.h"
#include <memory>
#include <vector>

class CompilationUnitSyntaxNode : public StatementSyntaxNode {
public:
  CompilationUnitSyntaxNode(
      std::vector<std::unique_ptr<StatementSyntaxNode>> statements,
      std::shared_ptr<SyntaxToken> endOfFileToken);
  CompilationUnitSyntaxNode(std::unique_ptr<StatementSyntaxNode> statement,
                            std::shared_ptr<SyntaxToken> endOfFileToken);
  virtual ~CompilationUnitSyntaxNode() {}

  const StatementSyntaxNode *Statement() const;
  const TemplateList<std::unique_ptr<StatementSyntaxNode>> &Statements() const;
  std::shared_ptr<SyntaxToken> EndOfFileToken();

private:
  TemplateList<std::unique_ptr<StatementSyntaxNode>> mStatements;
  std::shared_ptr<SyntaxToken> mEndOfFileToken;
};
