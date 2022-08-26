
#include "AssignmentExpressionNode.h"

AssignmentExpressionNode::AssignmentExpressionNode(
    std::shared_ptr<SyntaxToken> identifierToken,
    std::shared_ptr<SyntaxToken> equalsToken,
    std::unique_ptr<ExpressionNode> expression)
    : ExpressionNode(SyntaxType::AssignmentExpression),
      mIdentifier(identifierToken), mAssignmentOperator(equalsToken),
      mExpression(std::move(expression)) {
  mVecExpressionNodes.push_back(identifierToken.get());
  mVecExpressionNodes.push_back(mAssignmentOperator.get());
  mVecExpressionNodes.push_back(mExpression.get());
}

std::shared_ptr<SyntaxToken> AssignmentExpressionNode::IdentifierToken() {
  return mIdentifier;
}

std::shared_ptr<SyntaxToken> AssignmentExpressionNode::EqualsToken() {
  return mAssignmentOperator;
}

ExpressionNode *AssignmentExpressionNode::Expression() {
  return mExpression.get();
}