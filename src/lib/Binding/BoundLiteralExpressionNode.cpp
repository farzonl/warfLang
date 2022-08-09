#include "BoundLiteralExpressionNode.h"
#include "ValueType.h"

BoundLiteralExpressionNode::BoundLiteralExpressionNode(LiteralExpressionNode* literal) : mValue(const_cast<Value&>(literal->LiteralToken()->GetValue())) {
}

BoundNodeType BoundLiteralExpressionNode::NodeType() {
    return BoundNodeType::LiteralExpression;
}

Type BoundLiteralExpressionNode::GetType() {
    return mValue.GetType();
}

Value BoundLiteralExpressionNode::GetValue() {
    return mValue;
}