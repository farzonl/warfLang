/*
 * Copyright (c) 2022 F. Lotfi All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#pragma once

#include "Binding/BoundBinaryExpressionNode.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "Error/Record.h"

class Value;
class BoundFunctionDeclarationNode;

class Evaluator {
public:
  Evaluator(std::unique_ptr<BoundExpressionNode> root);
  Evaluator(std::unique_ptr<BoundStatementNode> root);
  Value Evaluate();
  BoundExpressionNode *Root() const;

private:
  Value EvaluateRec(BoundExpressionNode *node);
  Value EvaluateStatement(BoundStatementNode *node);
  std::unique_ptr<BoundNode> mRoot;
  std::unordered_map<std::string, const BoundFunctionDeclarationNode *>
      mFunctions;
};
