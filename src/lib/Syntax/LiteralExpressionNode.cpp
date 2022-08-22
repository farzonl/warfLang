// Copyright (c) 2022 F. Lotfi All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "LiteralExpressionNode.h"

LiteralExpressionNode::LiteralExpressionNode(
    std::shared_ptr<SyntaxToken> literalToken)
    : ExpressionNode(SyntaxType::LiteralExpression),
      mLiteralToken(literalToken) {
  mVecExpressionNodes.push_back(literalToken.get());
}
