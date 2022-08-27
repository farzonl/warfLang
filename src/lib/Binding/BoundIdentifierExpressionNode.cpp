// Copyright (c) 2022 F. Lotfi All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "BoundIdentifierExpressionNode.h"

BoundNodeType BoundIdentifierExpressionNode::NodeType() {
  return BoundNodeType::AssignmentExpression;
}

Type BoundIdentifierExpressionNode::GetType() { return Type::Unknown; }

BoundIdentifierExpressionNode::BoundIdentifierExpressionNode(
    const std::string &name)
    : BoundExpressionNode(), mName(name) {}