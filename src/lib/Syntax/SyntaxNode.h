#pragma once
#include <memory>
#include <vector>

#include "SyntaxType.h"

class SyntaxNode {
protected:
  SyntaxType mType;
  std::vector<SyntaxNode *> mVecExpressionNodes;

public:
  SyntaxNode(SyntaxType type);
  SyntaxType Type() { return mType; }
  virtual const std::vector<SyntaxNode *> &GetChildren();
};