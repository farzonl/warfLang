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
  Evaluator(std::unique_ptr<BoundExpressionNode> root, uint64_t maxSteps = 0);
  Evaluator(std::unique_ptr<BoundStatementNode> root, uint64_t maxSteps = 0);
  Value Evaluate(uint64_t maxSteps = 0);
  BoundExpressionNode *Root() const;

  void SetStepLimit(uint64_t limit) { mMaxSteps = limit; }
  void SetMaxSteps(uint64_t limit) { mMaxSteps = limit; }
  uint64_t StepLimit() const { return mMaxSteps; }
  uint64_t MaxSteps() const { return mMaxSteps; }
  uint64_t StepCount() const { return mStepCount; }
  void ResetStepCount() { mStepCount = 0; }

private:
  Value EvaluateRec(BoundExpressionNode *node);
  Value EvaluateStatement(BoundStatementNode *node);
  void SetVariableValue(VariableSymbol *variable, Value value);
  void CheckStepBudget();
  std::unique_ptr<BoundNode> mRoot;
  uint64_t mStepCount = 0;
  uint64_t mMaxSteps = 0;
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
