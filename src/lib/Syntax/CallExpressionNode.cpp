// Copyright (c) 2026 F. Lotfi All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "CallExpressionNode.h"

CallExpressionNode::CallExpressionNode(
    std::shared_ptr<SyntaxToken> identifier,
    std::shared_ptr<SyntaxToken> openParenthesis,
    std::vector<std::unique_ptr<ExpressionNode>> arguments,
    std::vector<std::shared_ptr<SyntaxToken>> commas,
    std::shared_ptr<SyntaxToken> closeParenthesis)
    : ExpressionNode(SyntaxKind::CallExpression), mIdentifier(identifier),
      mOpenParenthesis(openParenthesis), mArguments(std::move(arguments)),
      mCommas(std::move(commas)), mCloseParenthesis(closeParenthesis) {
  mVecExpressionNodes.push_back(mIdentifier.get());
  mVecExpressionNodes.push_back(mOpenParenthesis.get());
  for (size_t i = 0; i < mArguments.size(); i++) {
    mVecExpressionNodes.push_back(mArguments[i].get());
    if (i < mCommas.size()) {
      mVecExpressionNodes.push_back(mCommas[i].get());
    }
  }
  mVecExpressionNodes.push_back(mCloseParenthesis.get());
}

std::shared_ptr<SyntaxToken> CallExpressionNode::IdentifierToken() {
  return mIdentifier;
}

const std::vector<std::unique_ptr<ExpressionNode>> &
CallExpressionNode::Arguments() const {
  return mArguments;
}