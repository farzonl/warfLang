// Copyright (c) 2022 F. Lotfi All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "BoundLiteralExpressionNode.h"
#include "ValueType.h"

BoundLiteralExpressionNode::BoundLiteralExpressionNode(
    LiteralExpressionNode *literal)
    : mValue(const_cast<Value &>(literal->LiteralToken()->GetValue())) {}

BoundNodeType BoundLiteralExpressionNode::NodeType() {
  return BoundNodeType::LiteralExpression;
}

Type BoundLiteralExpressionNode::GetType() { return mValue.GetType(); }

Value BoundLiteralExpressionNode::GetValue() { return mValue; }