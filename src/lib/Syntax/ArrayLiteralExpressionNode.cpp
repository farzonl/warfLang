// Copyright (c) 2026 F. Lotfi All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "ArrayLiteralExpressionNode.h"

ArrayLiteralExpressionNode::ArrayLiteralExpressionNode(
    std::shared_ptr<SyntaxToken> openBracket,
    std::vector<std::unique_ptr<ExpressionNode>> elements,
    std::vector<std::shared_ptr<SyntaxToken>> commas,
    std::shared_ptr<SyntaxToken> closeBracket)
    : ExpressionNode(SyntaxKind::ArrayLiteralExpression),
      mOpenBracket(std::move(openBracket)), mElements(std::move(elements)),
      mCommas(std::move(commas)), mCloseBracket(std::move(closeBracket)) {
  mVecExpressionNodes.push_back(mOpenBracket.get());
  for (size_t i = 0; i < mElements.size(); i++) {
    mVecExpressionNodes.push_back(mElements[i].get());
    if (i < mCommas.size()) {
      mVecExpressionNodes.push_back(mCommas[i].get());
    }
  }
  mVecExpressionNodes.push_back(mCloseBracket.get());
}

std::shared_ptr<SyntaxToken> ArrayLiteralExpressionNode::OpenBracket() const {
  return mOpenBracket;
}

const std::vector<std::unique_ptr<ExpressionNode>> &
ArrayLiteralExpressionNode::Elements() const {
  return mElements;
}

std::shared_ptr<SyntaxToken> ArrayLiteralExpressionNode::CloseBracket() const {
  return mCloseBracket;
}
