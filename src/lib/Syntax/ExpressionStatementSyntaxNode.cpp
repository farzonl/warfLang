// Copyright (c) 2022 F. Lotfi All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "ExpressionStatementSyntaxNode.h"

ExpressionStatementSyntaxNode::ExpressionStatementSyntaxNode(
    std::unique_ptr<ExpressionNode> expression)
    : StatementSyntaxNode(SyntaxKind::ExpressionStatement),
      mExpression(std::move(expression)) {
        mVecExpressionNodes.push_back(mExpression.get());
      }

ExpressionNode *ExpressionStatementSyntaxNode::Expression() const {
  return mExpression.get();
}