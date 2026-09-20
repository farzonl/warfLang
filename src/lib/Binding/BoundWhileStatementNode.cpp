#include "BoundWhileStatementNode.h"

BoundWhileStatementNode::BoundWhileStatementNode(
    std::unique_ptr<BoundExpressionNode> condition,
    std::unique_ptr<BoundStatementNode> body)
    : mCondition(std::move(condition)), mBody(std::move(body)) {}

BoundNodeKind BoundWhileStatementNode::Kind() {
  return BoundNodeKind::WhileStatement;
}

const BoundExpressionNode *BoundWhileStatementNode::Condition() const {
  return mCondition.get();
}

const BoundStatementNode *BoundWhileStatementNode::Body() const {
  return mBody.get();
}
