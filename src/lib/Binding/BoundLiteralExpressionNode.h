#pragma once

#include <memory>

#include "BoundExpressionNode.h"
#include "Syntax/SyntaxType.h"

class BoundLiteralExpressionNode : public BoundExpressionNode {
    public:
        BoundLiteralExpressionNode(ValueType value);
        ValueType Value();
    private:
        ValueType mValue;
};