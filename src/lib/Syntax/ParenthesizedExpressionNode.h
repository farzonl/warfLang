/*
 * Copyright (c) 2022 F. Lotfi All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#pragma once
#include "ExpressionNode.h"
#include "SyntaxToken.h"

class ParenthesizedExpressionNode : public ExpressionNode {

  std::shared_ptr<SyntaxToken> mOpenParenToken;
  std::unique_ptr<ExpressionNode> mExpression;
  std::shared_ptr<SyntaxToken> mCloseParenToken;

public:
  ParenthesizedExpressionNode(std::shared_ptr<SyntaxToken> openParenToken,
                              std::unique_ptr<ExpressionNode> expression,
                              std::shared_ptr<SyntaxToken> closeParenToken);
  ExpressionNode *Expression() const { return mExpression.get(); }
  virtual ~ParenthesizedExpressionNode() {}
};