// Copyright (c) 2026 F. Lotfi All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "BoundIndexAssignmentExpressionNode.h"

BoundIndexAssignmentExpressionNode::BoundIndexAssignmentExpressionNode(
    std::unique_ptr<BoundExpressionNode> target,
    std::unique_ptr<BoundExpressionNode> index,
    std::unique_ptr<BoundExpressionNode> value)
    : mTarget(std::move(target)), mIndex(std::move(index)),
      mValue(std::move(value)) {}

BoundNodeKind BoundIndexAssignmentExpressionNode::Kind() {
  return BoundNodeKind::IndexAssignmentExpression;
}

Value::Type BoundIndexAssignmentExpressionNode::Type() {
  return mValue->Type();
}

BoundExpressionNode *BoundIndexAssignmentExpressionNode::Target() const {
  return mTarget.get();
}

BoundExpressionNode *BoundIndexAssignmentExpressionNode::Index() const {
  return mIndex.get();
}

BoundExpressionNode *
BoundIndexAssignmentExpressionNode::ValueExpression() const {
  return mValue.get();
}
