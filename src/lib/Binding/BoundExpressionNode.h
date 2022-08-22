#pragma once

#include "ValueType.h"

enum class BoundNodeType {

};

class BoundExpressionNode {
public:
  virtual BoundNodeType NodeType() = 0;
  virtual Type GetType() = 0;
};