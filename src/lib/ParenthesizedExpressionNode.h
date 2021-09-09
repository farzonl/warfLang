#pragma once
#include "ExpressionNode.h"
#include "SyntaxToken.h"

class ParenthesizedExpressionNode : public ExpressionNode {

    std::shared_ptr<SyntaxToken> mOpenParenToken;
    std::unique_ptr<ExpressionNode> mExpression;
    std::shared_ptr<SyntaxToken> mCloseParenToken;
    public:
        ParenthesizedExpressionNode(std::shared_ptr<SyntaxToken> openParenToken,
            std::unique_ptr<ExpressionNode> expression,
            std::shared_ptr<SyntaxToken> closeParenToken);
        ExpressionNode* Expression() const {return mExpression.get(); }
};