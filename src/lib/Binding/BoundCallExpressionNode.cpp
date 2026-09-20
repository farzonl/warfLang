// Copyright (c) 2026 F. Lotfi All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "BoundCallExpressionNode.h"

BoundCallExpressionNode::BoundCallExpressionNode(
    std::string name,
    std::vector<std::unique_ptr<BoundExpressionNode>> arguments,
    std::vector<std::shared_ptr<VariableSymbol>> parameters,
    std::unique_ptr<BoundStatementNode> body, Value::Type type)
    : BoundExpressionNode(), mName(std::move(name)),
      mArguments(std::move(arguments)), mParameters(std::move(parameters)),
      mBody(std::move(body)), mType(type) {}

BoundNodeKind BoundCallExpressionNode::Kind() {
  return BoundNodeKind::CallExpression;
}

Value::Type BoundCallExpressionNode::Type() { return mType; }

const std::string &BoundCallExpressionNode::Name() const { return mName; }

const std::vector<std::unique_ptr<BoundExpressionNode>> &
BoundCallExpressionNode::Arguments() const {
  return mArguments;
}

const std::vector<std::shared_ptr<VariableSymbol>> &
BoundCallExpressionNode::Parameters() const {
  return mParameters;
}

const BoundStatementNode *BoundCallExpressionNode::Body() const {
  return mBody.get();
}
