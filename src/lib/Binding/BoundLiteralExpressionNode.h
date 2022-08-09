#pragma once

#include <memory>

#include "BoundExpressionNode.h"
#include "Syntax/SyntaxType.h"
#include "Syntax/LiteralExpressionNode.h"

class BoundLiteralExpressionNode : public BoundExpressionNode {
    public:
        BoundLiteralExpressionNode(LiteralExpressionNode* literal);
        //Value GetValue();
        virtual BoundNodeType NodeType() override;
        virtual Type GetType() override; 
        Value GetValue(); 
    private:
        Value mValue;
};