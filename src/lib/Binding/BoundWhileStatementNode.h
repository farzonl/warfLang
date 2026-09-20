#pragma once

#include "BoundExpressionNode.h"
#include <memory>

class BoundWhileStatementNode : public BoundStatementNode {
public:
  BoundWhileStatementNode(std::unique_ptr<BoundExpressionNode> condition,
                          std::unique_ptr<BoundStatementNode> body);

  BoundNodeKind Kind() override;
  const BoundExpressionNode *Condition() const;
  const BoundStatementNode *Body() const;

private:
  std::unique_ptr<BoundExpressionNode> mCondition;
  std::unique_ptr<BoundStatementNode> mBody;
};
