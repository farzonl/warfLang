// Copyright (c) 2022 F. Lotfi All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "IdentifierExpressionNode.h"

IdentifierExpressionNode::IdentifierExpressionNode(
    std::shared_ptr<SyntaxToken> identifierToken)
    : ExpressionNode(SyntaxType::IdentifierExpression),
      mIdentifier(identifierToken) {
  mVecExpressionNodes.push_back(identifierToken.get());
}

std::shared_ptr<SyntaxToken> IdentifierExpressionNode::IdentifierToken() {
  return mIdentifier;
}