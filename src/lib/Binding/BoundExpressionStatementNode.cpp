/*
 * Copyright (c) 2022 F. Lotfi All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */
#include "BoundExpressionStatementNode.h"

BoundExpressionStatementNode::BoundExpressionStatementNode(
  std::unique_ptr<BoundExpressionNode> expression) : 
  BoundStatementNode(), 
  mExpression(std::move(expression)) {}
  
BoundNodeKind BoundExpressionStatementNode::Kind() {
  return BoundNodeKind::ExpressionStatement;
}
const BoundExpressionNode* BoundExpressionStatementNode::Expression() const {
  return mExpression.get();
}
