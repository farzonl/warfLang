// Copyright (c) 2026 F. Lotfi All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "BoundDoWhileStatementNode.h"

BoundDoWhileStatementNode::BoundDoWhileStatementNode(
    std::unique_ptr<BoundStatementNode> body,
    std::unique_ptr<BoundExpressionNode> condition)
    : mBody(std::move(body)), mCondition(std::move(condition)) {}

BoundNodeKind BoundDoWhileStatementNode::Kind() {
  return BoundNodeKind::DoWhileStatement;
}

const BoundStatementNode *BoundDoWhileStatementNode::Body() const {
  return mBody.get();
}

const BoundExpressionNode *BoundDoWhileStatementNode::Condition() const {
  return mCondition.get();
}
