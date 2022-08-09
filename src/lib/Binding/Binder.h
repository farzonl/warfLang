#pragma once

#include <memory>

#include "BoundExpressionNode.h"
#include "Syntax/ExpressionNode.h"

class LiteralExpressionNode;
class UnaryExpressionNode;
class BinaryExpressionNode;

class Binder {
    public:
        std::unique_ptr<BoundExpressionNode> BindExpression(ExpressionNode* syntax);
        const std::vector<std::string> &Diagnostics() const { return mDiagnostics; }
    private:
        std::vector<std::string> mDiagnostics;
        std::unique_ptr<BoundExpressionNode> BindLiteralExpression(LiteralExpressionNode* literal);
        std::unique_ptr<BoundExpressionNode> BindUnaryExpression(UnaryExpressionNode* unary);
        std::unique_ptr<BoundExpressionNode> BindBinaryExpression(BinaryExpressionNode* binary);
        
};