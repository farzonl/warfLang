// Copyright (c) 2026 F. Lotfi All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "BoundIndexExpressionNode.h"

BoundIndexExpressionNode::BoundIndexExpressionNode(
    std::unique_ptr<BoundExpressionNode> target,
    std::unique_ptr<BoundExpressionNode> index, Value::Type elementType)
    : mTarget(std::move(target)), mIndex(std::move(index)),
      mElementType(elementType) {}

BoundNodeKind BoundIndexExpressionNode::Kind() {
  return BoundNodeKind::IndexExpression;
}

Value::Type BoundIndexExpressionNode::Type() { return mElementType; }

BoundExpressionNode *BoundIndexExpressionNode::Target() const {
  return mTarget.get();
}

BoundExpressionNode *BoundIndexExpressionNode::Index() const {
  return mIndex.get();
}
