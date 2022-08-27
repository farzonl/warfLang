// Copyright (c) 2022 F. Lotfi All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "BoundAssignmentExpressionNode.h"

BoundNodeType BoundAssignmentExpressionNode::NodeType() {
  return BoundNodeType::AssignmentExpression;
}

Type BoundAssignmentExpressionNode::GetType() {
  return mBoundExpression->GetType();
}

BoundExpressionNode *BoundAssignmentExpressionNode::BoundExpression() {
  return mBoundExpression.get();
}
std::string BoundAssignmentExpressionNode::Identifier() { return mIdentifer; }

BoundAssignmentExpressionNode::BoundAssignmentExpressionNode(
    std::string identifer, std::unique_ptr<BoundExpressionNode> boundExpression)
    : BoundExpressionNode(), mIdentifer(identifer),
      mBoundExpression(std::move(boundExpression)) {}