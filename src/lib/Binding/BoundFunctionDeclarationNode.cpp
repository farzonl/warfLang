// Copyright (c) 2026 F. Lotfi All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "BoundFunctionDeclarationNode.h"

BoundFunctionDeclarationNode::BoundFunctionDeclarationNode(
    std::shared_ptr<FunctionSymbol> function,
    std::unique_ptr<BoundStatementNode> body)
    : BoundStatementNode(), mFunction(function), mBody(std::move(body)) {}

BoundNodeKind BoundFunctionDeclarationNode::Kind() {
  return BoundNodeKind::FunctionDeclaration;
}

std::shared_ptr<FunctionSymbol> BoundFunctionDeclarationNode::Function() const {
  return mFunction;
}

const BoundStatementNode *BoundFunctionDeclarationNode::Body() const {
  return mBody.get();
}