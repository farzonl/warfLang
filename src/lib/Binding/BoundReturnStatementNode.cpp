// Copyright (c) 2026 F. Lotfi All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "BoundReturnStatementNode.h"

BoundReturnStatementNode::BoundReturnStatementNode(
    std::unique_ptr<BoundExpressionNode> expression)
    : mExpression(std::move(expression)) {}

BoundNodeKind BoundReturnStatementNode::Kind() {
  return BoundNodeKind::ReturnStatement;
}

const BoundExpressionNode *BoundReturnStatementNode::Expression() const {
  return mExpression.get();
}
