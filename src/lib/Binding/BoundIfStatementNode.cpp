// Copyright (c) 2026 F. Lotfi All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "BoundIfStatementNode.h"

BoundIfStatementNode::BoundIfStatementNode(
    std::unique_ptr<BoundExpressionNode> condition,
    std::unique_ptr<BoundStatementNode> thenStatement,
    std::unique_ptr<BoundStatementNode> elseStatement)
    : mCondition(std::move(condition)),
      mThenStatement(std::move(thenStatement)),
      mElseStatement(std::move(elseStatement)) {}

BoundNodeKind BoundIfStatementNode::Kind() { return BoundNodeKind::IfStatement; }

const BoundExpressionNode *BoundIfStatementNode::Condition() const {
  return mCondition.get();
}

const BoundStatementNode *BoundIfStatementNode::ThenStatement() const {
  return mThenStatement.get();
}

const BoundStatementNode *BoundIfStatementNode::ElseStatement() const {
  return mElseStatement.get();
}
