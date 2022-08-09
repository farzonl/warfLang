#pragma once

#include "ValueType.h"

enum class BoundNodeType {
    Unknown,
    UnaryExpression,
    LiteralExpression
};

class BoundExpressionNode {
    public:
        virtual BoundNodeType NodeType() = 0;
        virtual Type GetType() = 0;
        virtual ~BoundExpressionNode() {}
};