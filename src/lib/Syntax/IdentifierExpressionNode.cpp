#include "IdentifierExpressionNode.h"

IdentifierExpressionNode::IdentifierExpressionNode(
    std::shared_ptr<SyntaxToken> identifierToken)
    : ExpressionNode(SyntaxType::IdentifierExpression),
      mIdentifier(identifierToken) {
  mVecExpressionNodes.push_back(identifierToken.get());
}

std::shared_ptr<SyntaxToken> IdentifierExpressionNode::IdentifierToken() {
  return mIdentifier;
}