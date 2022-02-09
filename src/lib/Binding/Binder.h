#pragma once

#include <memory>

#include "BoundExpressionNode.h"
#include "Syntax/ExpressionNode.h"

class LiteralExpressionNode;
class UnaryExpressionNode;
class BinaryExpressionNode;

class Binder {
    static std::unique_ptr<BoundExpressionNode> BindExpression(ExpressionNode* syntax);
    private:
        static std::unique_ptr<BoundExpressionNode> BindLiteralExpression(LiteralExpressionNode* literal);
        static std::unique_ptr<BoundExpressionNode> BindUnaryExpression(UnaryExpressionNode* unary);
        static std::unique_ptr<BoundExpressionNode> BindBinaryExpression(BinaryExpressionNode* binary);
        Binder() = delete;
};