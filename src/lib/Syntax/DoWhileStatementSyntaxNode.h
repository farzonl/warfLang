/*
 * Copyright (c) 2026 F. Lotfi All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#pragma once

#include "ExpressionNode.h"
#include "StatementSyntaxNode.h"
#include "SyntaxToken.h"
#include <memory>

class DoWhileStatementSyntaxNode : public StatementSyntaxNode {
public:
  DoWhileStatementSyntaxNode(
      std::shared_ptr<SyntaxToken> doKeyword,
      std::unique_ptr<StatementSyntaxNode> body,
      std::shared_ptr<SyntaxToken> whileKeyword,
      std::shared_ptr<SyntaxToken> openParenthesisToken,
      std::unique_ptr<ExpressionNode> condition,
      std::shared_ptr<SyntaxToken> closeParenthesisToken);

  const StatementSyntaxNode *Body() const;
  std::unique_ptr<ExpressionNode> &Condition();

private:
  std::shared_ptr<SyntaxToken> mDoKeyword;
  std::unique_ptr<StatementSyntaxNode> mBody;
  std::shared_ptr<SyntaxToken> mWhileKeyword;
  std::shared_ptr<SyntaxToken> mOpenParenthesisToken;
  std::unique_ptr<ExpressionNode> mCondition;
  std::shared_ptr<SyntaxToken> mCloseParenthesisToken;
};
