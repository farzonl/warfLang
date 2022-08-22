#pragma once

#include "Binding\BoundExpressionNode.h"

class Evaluator {
public:
  Evaluator(std::unique_ptr<BoundExpressionNode> root);
  Value Evaluator::Evaluate();
  BoundExpressionNode *Root() const;
  const std::vector<std::string> &Errors() const { return mVecErrors; }

private:
  Value EvaluateRec(BoundExpressionNode *node);
  std::unique_ptr<BoundExpressionNode> mRootExpression;
  std::vector<std::string> mVecErrors;
};