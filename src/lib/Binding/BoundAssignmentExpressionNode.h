#pragma once

#include "BoundExpressionNode.h"

class BoundAssignmentExpressionNode : public BoundExpressionNode {
public:
  BoundAssignmentExpressionNode(
      std::string identifierToken,
      std::unique_ptr<BoundExpressionNode> boundExpression);

  virtual BoundNodeType NodeType() override;
  BoundExpressionNode *BoundExpression();
  std::string Identifier();
  virtual Type GetType() override;

private:
  std::unique_ptr<BoundExpressionNode> mBoundExpression;
  std::string mIdentifer;
};
