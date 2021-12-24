#pragma once
#include "ExpressionNode.h"
#include "SyntaxToken.h"

class LiteralExpression : public ExpressionNode {
  std::shared_ptr<SyntaxToken> mLiteralToken;

public:
  LiteralExpression(std::shared_ptr<SyntaxToken> literalToken);
  std::shared_ptr<SyntaxToken> LiteralToken() { return mLiteralToken; }
};