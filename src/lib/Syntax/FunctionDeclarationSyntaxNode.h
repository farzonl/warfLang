/*
 * Copyright (c) 2026 F. Lotfi All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#pragma once

#include "BlockStatementSyntaxNode.h"
#include "StatementSyntaxNode.h"
#include "SyntaxToken.h"
#include <memory>
#include <vector>

class FunctionDeclarationSyntaxNode : public StatementSyntaxNode {
public:
  FunctionDeclarationSyntaxNode(
      std::shared_ptr<SyntaxToken> functionKeyword,
      std::shared_ptr<SyntaxToken> identifier,
      std::shared_ptr<SyntaxToken> openParenthesis,
      std::vector<std::shared_ptr<SyntaxToken>> parameters,
      std::vector<std::shared_ptr<SyntaxToken>> commas,
      std::shared_ptr<SyntaxToken> closeParenthesis,
      std::unique_ptr<BlockStatementSyntaxNode> body);
  virtual ~FunctionDeclarationSyntaxNode() {}

  std::shared_ptr<SyntaxToken> Identifier();
  const std::vector<std::shared_ptr<SyntaxToken>> &Parameters() const;
  const BlockStatementSyntaxNode *Body() const;

private:
  std::shared_ptr<SyntaxToken> mFunctionKeyword;
  std::shared_ptr<SyntaxToken> mIdentifier;
  std::shared_ptr<SyntaxToken> mOpenParenthesis;
  std::vector<std::shared_ptr<SyntaxToken>> mParameters;
  std::vector<std::shared_ptr<SyntaxToken>> mCommas;
  std::shared_ptr<SyntaxToken> mCloseParenthesis;
  std::unique_ptr<BlockStatementSyntaxNode> mBody;
};