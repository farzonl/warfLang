/*
 * Copyright (c) 2022 F. Lotfi All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#pragma once

#include "Binding/BoundBinaryExpressionNode.h"
#include <unordered_map>
#include <vector>

class Value;

class Evaluator {
public:
  Evaluator(std::unique_ptr<BoundExpressionNode> root);
  Value Evaluate();
  BoundExpressionNode *Root() const;
  const std::vector<std::string> &Errors() const { return mVecErrors; }

private:
  Value EvaluateRec(BoundExpressionNode *node);
  std::unique_ptr<BoundExpressionNode> mRootExpression;
  std::vector<std::string> mVecErrors;
};