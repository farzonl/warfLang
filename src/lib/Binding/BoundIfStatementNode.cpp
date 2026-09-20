#include "BoundIfStatementNode.h"

BoundIfStatementNode::BoundIfStatementNode(
    std::unique_ptr<BoundExpressionNode> condition,
    std::unique_ptr<BoundStatementNode> thenStatement,
    std::unique_ptr<BoundStatementNode> elseStatement)
    : mCondition(std::move(condition)),
      mThenStatement(std::move(thenStatement)),
      mElseStatement(std::move(elseStatement)) {}

BoundNodeKind BoundIfStatementNode::Kind() { return BoundNodeKind::IfStatement; }

const BoundExpressionNode *BoundIfStatementNode::Condition() const {
  return mCondition.get();
}

const BoundStatementNode *BoundIfStatementNode::ThenStatement() const {
  return mThenStatement.get();
}

const BoundStatementNode *BoundIfStatementNode::ElseStatement() const {
  return mElseStatement.get();
}
