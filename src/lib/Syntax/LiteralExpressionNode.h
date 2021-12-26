#pragma once
#include "ExpressionNode.h"
#include "SyntaxToken.h"

class LiteralExpressionNode : public ExpressionNode {
  std::shared_ptr<SyntaxToken> mLiteralToken;

public:
  LiteralExpressionNode(std::shared_ptr<SyntaxToken> literalToken);
  std::shared_ptr<SyntaxToken> LiteralToken() { return mLiteralToken; }
};