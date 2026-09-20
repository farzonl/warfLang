// Copyright (c) 2026 F. Lotfi All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "FunctionDeclarationSyntaxNode.h"

FunctionDeclarationSyntaxNode::FunctionDeclarationSyntaxNode(
    std::shared_ptr<SyntaxToken> functionKeyword,
    std::shared_ptr<SyntaxToken> identifier,
    std::shared_ptr<SyntaxToken> openParenthesis,
    std::vector<std::shared_ptr<SyntaxToken>> parameters,
    std::vector<std::shared_ptr<SyntaxToken>> commas,
    std::shared_ptr<SyntaxToken> closeParenthesis,
    std::unique_ptr<BlockStatementSyntaxNode> body)
    : StatementSyntaxNode(SyntaxKind::FunctionDeclaration),
      mFunctionKeyword(functionKeyword), mIdentifier(identifier),
      mOpenParenthesis(openParenthesis), mParameters(std::move(parameters)),
      mCommas(std::move(commas)), mCloseParenthesis(closeParenthesis),
      mBody(std::move(body)) {
  mVecExpressionNodes.push_back(mFunctionKeyword.get());
  mVecExpressionNodes.push_back(mIdentifier.get());
  mVecExpressionNodes.push_back(mOpenParenthesis.get());
  for (size_t i = 0; i < mParameters.size(); i++) {
    mVecExpressionNodes.push_back(mParameters[i].get());
    if (i < mCommas.size()) {
      mVecExpressionNodes.push_back(mCommas[i].get());
    }
  }
  mVecExpressionNodes.push_back(mCloseParenthesis.get());
  mVecExpressionNodes.push_back(mBody.get());
}

std::shared_ptr<SyntaxToken> FunctionDeclarationSyntaxNode::Identifier() {
  return mIdentifier;
}

const std::vector<std::shared_ptr<SyntaxToken>> &
FunctionDeclarationSyntaxNode::Parameters() const {
  return mParameters;
}

const BlockStatementSyntaxNode *FunctionDeclarationSyntaxNode::Body() const {
  return mBody.get();
}
