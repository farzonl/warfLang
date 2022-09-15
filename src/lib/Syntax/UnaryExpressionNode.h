/*
 * Copyright (c) 2022 F. Lotfi All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#pragma once
#include "ExpressionNode.h"
#include "SyntaxToken.h"

class UnaryExpressionNode : public ExpressionNode {
  std::unique_ptr<ExpressionNode> mOperand;
  std::shared_ptr<SyntaxToken> mOperator;

public:
  UnaryExpressionNode(std::shared_ptr<SyntaxToken> operatorToken,
                      std::unique_ptr<ExpressionNode> operand);

  ExpressionNode *Operand();
  std::shared_ptr<SyntaxToken> Operator();
  virtual ~UnaryExpressionNode() {}
};