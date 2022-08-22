/*
 * Copyright (c) 2022 F. Lotfi All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#pragma once
#include "ExpressionNode.h"
#include "SyntaxToken.h"

class BinaryExpressionNode : public ExpressionNode {
  std::unique_ptr<ExpressionNode> mLeft;
  std::unique_ptr<ExpressionNode> mRight;
  std::shared_ptr<SyntaxToken> mOperator;

public:
  BinaryExpressionNode(std::unique_ptr<ExpressionNode> left,
                       std::shared_ptr<SyntaxToken> operatorToken,
                       std::unique_ptr<ExpressionNode> right);
  ExpressionNode *Left();
  ExpressionNode *Right();
  SyntaxToken *Operator();
};