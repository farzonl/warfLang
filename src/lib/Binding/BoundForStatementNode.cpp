#include "BoundForStatementNode.h"

BoundForStatementNode::BoundForStatementNode(
    std::unique_ptr<BoundStatementNode> initializer,
    std::unique_ptr<BoundExpressionNode> condition,
    std::unique_ptr<BoundExpressionNode> increment,
    std::unique_ptr<BoundStatementNode> body)
    : mInitializer(std::move(initializer)), mCondition(std::move(condition)),
      mIncrement(std::move(increment)), mBody(std::move(body)) {}

BoundNodeKind BoundForStatementNode::Kind() { return BoundNodeKind::ForStatement; }

const BoundStatementNode *BoundForStatementNode::Initializer() const {
  return mInitializer.get();
}

const BoundExpressionNode *BoundForStatementNode::Condition() const {
  return mCondition.get();
}

const BoundExpressionNode *BoundForStatementNode::Increment() const {
  return mIncrement.get();
}

const BoundStatementNode *BoundForStatementNode::Body() const {
  return mBody.get();
}
