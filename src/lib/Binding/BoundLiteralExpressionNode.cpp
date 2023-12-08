// Copyright (c) 2022 F. Lotfi All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "BoundLiteralExpressionNode.h"
#include "ValueType.h"

BoundLiteralExpressionNode::BoundLiteralExpressionNode(
    LiteralExpressionNode *literal)
    : mValue(const_cast<Value &>(literal->LiteralToken()->GetValue())) {}

BoundLiteralExpressionNode::BoundLiteralExpressionNode(int32_t v) : mValue() {
  mValue = 0;
}

BoundNodeKind BoundLiteralExpressionNode::Kind() {
  return BoundNodeKind::LiteralExpression;
}

Value::Type BoundLiteralExpressionNode::Type() { return mValue.VType(); }

Value BoundLiteralExpressionNode::GetValue() { return mValue; }