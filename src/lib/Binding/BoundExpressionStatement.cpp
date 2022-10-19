#include "BoundExpressionStatement.h"

BoundExpressionStatement::BoundExpressionStatement(
    std::unique_ptr<BoundExpressionNode> expression)
    : BoundStatement(), mExpression(std::move(expression)) {}

BoundExpressionNode *BoundExpressionStatement::Expression() {
  return mExpression.get();
}

BoundNodeType BoundExpressionStatement::NodeType() {
  return BoundNodeType::ExpressionStatement;
}