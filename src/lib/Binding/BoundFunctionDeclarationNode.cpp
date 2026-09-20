// Copyright (c) 2026 F. Lotfi All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "BoundFunctionDeclarationNode.h"

BoundFunctionDeclarationNode::BoundFunctionDeclarationNode(
    std::shared_ptr<FunctionSymbol> function)
    : BoundStatementNode(), mFunction(function) {}

BoundNodeKind BoundFunctionDeclarationNode::Kind() {
  return BoundNodeKind::FunctionDeclaration;
}

std::shared_ptr<FunctionSymbol> BoundFunctionDeclarationNode::Function() const {
  return mFunction;
}
