/*
 * Copyright (c) 2022 F. Lotfi All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#pragma once
#include "StatementSyntaxNode.h"
#include "ExpressionNode.h"
#include "SyntaxToken.h"
#include <memory>
#include <vector>

class VariableDeclarationSyntaxNode  : public StatementSyntaxNode {
public:
  VariableDeclarationSyntaxNode( std::shared_ptr<SyntaxToken> keyword,
   std::shared_ptr<SyntaxToken> identifier,
   std::shared_ptr<SyntaxToken> equalsToken,
   std::unique_ptr<ExpressionNode> initializer);

  virtual ~VariableDeclarationSyntaxNode() {}

  std::shared_ptr<SyntaxToken> Keyword();
  std::shared_ptr<SyntaxToken> Identifier();
  std::shared_ptr<SyntaxToken> EqualsToken();
  const ExpressionNode* Initializer() const;
private:
  std::shared_ptr<SyntaxToken> mKeyword;
  std::shared_ptr<SyntaxToken> mIdentifier;
  std::shared_ptr<SyntaxToken> mEqualsToken;
  std::unique_ptr<ExpressionNode> mInitializer;
};