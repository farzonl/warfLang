#include "BoundIdentifierExpressionNode.h"

BoundNodeType BoundIdentifierExpressionNode::NodeType() {
  return BoundNodeType::AssignmentExpression;
}

Type BoundIdentifierExpressionNode::GetType() {
  return Type::Unknown;
}

BoundIdentifierExpressionNode::BoundIdentifierExpressionNode(const std::string& name)
    : BoundExpressionNode(), mName(name) {}