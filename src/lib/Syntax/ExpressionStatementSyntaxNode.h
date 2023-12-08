/*
 * Copyright (c) 2022 F. Lotfi All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#pragma once
#include "ExpressionNode.h"
#include "StatementSyntaxNode.h"

class ExpressionStatementSyntaxNode : public StatementSyntaxNode {
public:
  ExpressionStatementSyntaxNode(std::unique_ptr<ExpressionNode> expression);
  virtual ~ExpressionStatementSyntaxNode() {}
  ExpressionNode *Expression() const;

private:
  std::unique_ptr<ExpressionNode> mExpression;
};