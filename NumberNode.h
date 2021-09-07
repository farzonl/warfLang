#pragma once
#include "ExpressionNode.h"
#include "SyntaxToken.h"

class NumberNode : public ExpressionNode {
    std::shared_ptr<SyntaxToken> mNumberToken;
    public:
        NumberNode(std::shared_ptr<SyntaxToken> numberToken);
        std::shared_ptr<SyntaxToken> NumberToken() { return mNumberToken; }
};