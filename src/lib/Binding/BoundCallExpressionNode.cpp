// Copyright (c) 2026 F. Lotfi All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "BoundCallExpressionNode.h"

BoundCallExpressionNode::BoundCallExpressionNode(
    std::string name,
    std::vector<std::unique_ptr<BoundExpressionNode>> arguments,
    Value::Type type)
    : BoundExpressionNode(), mName(std::move(name)),
      mArguments(std::move(arguments)), mType(type) {}

BoundNodeKind BoundCallExpressionNode::Kind() {
  return BoundNodeKind::CallExpression;
}

Value::Type BoundCallExpressionNode::Type() { return mType; }

const std::string &BoundCallExpressionNode::Name() const { return mName; }

const std::vector<std::unique_ptr<BoundExpressionNode>> &
BoundCallExpressionNode::Arguments() const {
  return mArguments;
}
