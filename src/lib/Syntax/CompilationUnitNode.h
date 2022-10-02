/*
 * Copyright (c) 2022 F. Lotfi All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#pragma once
#include "ExpressionNode.h"
#include "SyntaxNode.h"

class CompilationUnitNode : public SyntaxNode {
private:
  std::unique_ptr<ExpressionNode> mExpression;

public:
  CompilationUnitNode(std::unique_ptr<ExpressionNode> expression);
  virtual ~CompilationUnitNode() {}
  ExpressionNode *Expression() const;
};