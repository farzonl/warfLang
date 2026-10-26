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
class BoundCallExpressionNode;
class VariableSymbol;

class Evaluator {
public:
  Evaluator(std::unique_ptr<BoundExpressionNode> root);
  Evaluator(std::unique_ptr<BoundStatementNode> root);
  Value Evaluate();
  BoundExpressionNode *Root() const;

private:
  Value EvaluateRec(BoundExpressionNode *node);
  Value EvaluateStatement(BoundStatementNode *node);
  void SetVariableValue(VariableSymbol *variable, Value value);
  std::unique_ptr<BoundNode> mRoot;
  std::unordered_map<std::string, const BoundFunctionDeclarationNode *>
      mFunctions;
  std::unordered_map<std::string, const BoundCallExpressionNode *>
      mActiveCallNodes;
  std::vector<std::unordered_map<VariableSymbol *, Value>> mCallFrames;
  bool mHasReturnValue = false;
  bool mShouldBreak = false;
  bool mShouldContinue = false;
  Value mReturnValue;
};
