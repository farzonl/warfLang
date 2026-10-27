// Copyright (c) 2022 F. Lotfi All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "VariableDeclarationSyntaxNode.h"

VariableDeclarationSyntaxNode::VariableDeclarationSyntaxNode(
    std::shared_ptr<SyntaxToken> keyword,
    std::shared_ptr<SyntaxToken> identifier,
    std::shared_ptr<SyntaxToken> equalsToken,
    std::unique_ptr<ExpressionNode> initializer, Value::Type typeAnnotation)
    : StatementSyntaxNode(SyntaxKind::VariableDeclaration),
      mKeyword(std::move(keyword)), mIdentifier(std::move(identifier)),
      mEqualsToken(std::move(equalsToken)),
      mInitializer(std::move(initializer)), mTypeAnnotation(typeAnnotation) {
  mVecExpressionNodes.push_back(mKeyword.get());
  mVecExpressionNodes.push_back(mIdentifier.get());
  mVecExpressionNodes.push_back(mEqualsToken.get());
  mVecExpressionNodes.push_back(mInitializer.get());
}

std::shared_ptr<SyntaxToken> VariableDeclarationSyntaxNode::Keyword() {
  return mKeyword;
}
std::shared_ptr<SyntaxToken> VariableDeclarationSyntaxNode::Identifier() {
  return mIdentifier;
}

std::shared_ptr<SyntaxToken> VariableDeclarationSyntaxNode::EqualsToken() {
  return mEqualsToken;
}
const ExpressionNode *VariableDeclarationSyntaxNode::Initializer() const {
  return mInitializer.get();
}
Value::Type VariableDeclarationSyntaxNode::TypeAnnotation() const {
  return mTypeAnnotation;
}
