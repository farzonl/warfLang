// Copyright (c) 2022 F. Lotfi All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "VariableDeclarationSyntaxNode.h"

VariableDeclarationSyntaxNode::VariableDeclarationSyntaxNode( std::shared_ptr<SyntaxToken> keyword,
   std::shared_ptr<SyntaxToken> identifier,
   std::shared_ptr<SyntaxToken> equalsToken,
   std::unique_ptr<ExpressionNode> initializer) :   StatementSyntaxNode(SyntaxKind::VariableDeclaration),
   mIdentifier(identifier), mEqualsToken(equalsToken), mInitializer(std::move(initializer)) {}

  std::shared_ptr<SyntaxToken> VariableDeclarationSyntaxNode::Keyword() {
    return mKeyword;
  }
  std::shared_ptr<SyntaxToken> VariableDeclarationSyntaxNode::Identifier() {
    return mIdentifier;
  }

  std::shared_ptr<SyntaxToken> VariableDeclarationSyntaxNode::EqualsToken() {
    return mEqualsToken;
  }
  const ExpressionNode* VariableDeclarationSyntaxNode::Initializer() const {
    return mInitializer.get();
  }