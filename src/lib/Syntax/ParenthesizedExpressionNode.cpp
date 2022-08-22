// Copyright (c) 2022 F. Lotfi All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "ParenthesizedExpressionNode.h"

ParenthesizedExpressionNode::ParenthesizedExpressionNode(
    std::shared_ptr<SyntaxToken> openParenToken,
    std::unique_ptr<ExpressionNode> expression,
    std::shared_ptr<SyntaxToken> closeParenToken)
    : ExpressionNode(SyntaxType::ParenthesizedExpression),
      mOpenParenToken(openParenToken), mExpression(std::move(expression)),
      mCloseParenToken(closeParenToken) {
  mVecExpressionNodes.push_back(mOpenParenToken.get());
  mVecExpressionNodes.push_back(mExpression.get());
  mVecExpressionNodes.push_back(mCloseParenToken.get());
}
