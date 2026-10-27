// Copyright (c) 2026 F. Lotfi All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "BoundArrayLiteralExpressionNode.h"

BoundArrayLiteralExpressionNode::BoundArrayLiteralExpressionNode(
    std::vector<std::unique_ptr<BoundExpressionNode>> elements,
    Value::Type elementType)
    : mElements(std::move(elements)), mElementType(elementType) {}

BoundNodeKind BoundArrayLiteralExpressionNode::Kind() {
  return BoundNodeKind::ArrayLiteralExpression;
}

Value::Type BoundArrayLiteralExpressionNode::Type() {
  return Value::Type::Array;
}

const std::vector<std::unique_ptr<BoundExpressionNode>> &
BoundArrayLiteralExpressionNode::Elements() const {
  return mElements;
}
