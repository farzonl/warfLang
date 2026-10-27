// Copyright (c) 2026 F. Lotfi All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "IndexExpressionNode.h"

IndexExpressionNode::IndexExpressionNode(
    std::unique_ptr<ExpressionNode> target,
    std::shared_ptr<SyntaxToken> openBracket,
    std::unique_ptr<ExpressionNode> index,
    std::shared_ptr<SyntaxToken> closeBracket)
    : ExpressionNode(SyntaxKind::IndexExpression), mTarget(std::move(target)),
      mOpenBracket(std::move(openBracket)), mIndex(std::move(index)),
      mCloseBracket(std::move(closeBracket)) {
  mVecExpressionNodes.push_back(mTarget.get());
  mVecExpressionNodes.push_back(mOpenBracket.get());
  mVecExpressionNodes.push_back(mIndex.get());
  mVecExpressionNodes.push_back(mCloseBracket.get());
}

ExpressionNode *IndexExpressionNode::Target() const { return mTarget.get(); }

std::shared_ptr<SyntaxToken> IndexExpressionNode::OpenBracket() const {
  return mOpenBracket;
}

ExpressionNode *IndexExpressionNode::Index() const { return mIndex.get(); }

std::shared_ptr<SyntaxToken> IndexExpressionNode::CloseBracket() const {
  return mCloseBracket;
}
