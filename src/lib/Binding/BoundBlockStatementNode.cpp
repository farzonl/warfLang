/*
 * Copyright (c) 2022 F. Lotfi All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */
#include "BoundBlockStatementNode.h"

BoundBlockStatementNode::BoundBlockStatementNode(
    TemplateList<std::unique_ptr<BoundStatementNode>> statements)
    : BoundStatementNode(), mStatements(std::move(statements)) {}

BoundNodeKind BoundBlockStatementNode::Kind() {
  return BoundNodeKind::BlockStatement;
}
const TemplateList<std::unique_ptr<BoundStatementNode>> &
BoundBlockStatementNode::Statements() const {
  return mStatements;
}
