/*
 * Copyright (c) 2022 F. Lotfi All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#pragma once
#include "StatementSyntaxNode.h"
#include "SyntaxToken.h"

class CompilationUnitSyntaxNode : public StatementSyntaxNode {
public:
  CompilationUnitSyntaxNode(std::unique_ptr<StatementSyntaxNode> statement,
                            std::shared_ptr<SyntaxToken> endOfFileToken);
  virtual ~CompilationUnitSyntaxNode() {}

  const StatementSyntaxNode *Statement() const;
  std::shared_ptr<SyntaxToken> EndOfFileToken();

private:
  std::unique_ptr<StatementSyntaxNode> mStatement;
  std::shared_ptr<SyntaxToken> mEndOfFileToken;
};