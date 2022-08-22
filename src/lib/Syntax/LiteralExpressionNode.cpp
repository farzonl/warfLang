#include "LiteralExpressionNode.h"

LiteralExpressionNode::LiteralExpressionNode(
    std::shared_ptr<SyntaxToken> literalToken)
    : ExpressionNode(SyntaxType::LiteralExpression),
      mLiteralToken(literalToken) {
  mVecExpressionNodes.push_back(literalToken.get());
}
