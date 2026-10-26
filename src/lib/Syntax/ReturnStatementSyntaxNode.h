/*
 * Copyright (c) 2026 F. Lotfi All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#pragma once

#include "ExpressionNode.h"
#include "StatementSyntaxNode.h"
#include "SyntaxToken.h"
#include <memory>

class ReturnStatementSyntaxNode : public StatementSyntaxNode {
public:
  ReturnStatementSyntaxNode(std::shared_ptr<SyntaxToken> returnKeyword,
                            std::unique_ptr<ExpressionNode> expression);
  virtual ~ReturnStatementSyntaxNode() {}

  std::shared_ptr<SyntaxToken> ReturnKeyword() const;
  const ExpressionNode *Expression() const;

private:
  std::shared_ptr<SyntaxToken> mReturnKeyword;
  std::unique_ptr<ExpressionNode> mExpression;
};
