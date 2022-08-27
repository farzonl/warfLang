#pragma once

#include "BoundExpressionNode.h"
#include <string>

class BoundIdentifierExpressionNode : public BoundExpressionNode {
public:
  BoundIdentifierExpressionNode(const std::string &name);

  virtual BoundNodeType NodeType() override;
  virtual Type GetType() override;
  std::string Name() { return mName; }

private:
  std::string mName;
};
