#pragma once
#include <memory>

#include "ExpressionNode.h"
#include "SyntaxToken.h"
#include "SyntaxTree.h"

class Parser {
    std::vector<std::shared_ptr<SyntaxToken>> mTokens;
    int32_t mPosition;
    std::vector<std::string> mVecErrors;
    std::shared_ptr<SyntaxToken> Peek(int32_t offset);
    std::shared_ptr<SyntaxToken> Current();
    std::shared_ptr<SyntaxToken> Next();
    std::shared_ptr<SyntaxToken> Match(SyntaxType type);

    std::unique_ptr<ExpressionNode> ParseTerm();
    std::unique_ptr<ExpressionNode> ParseFactor();
    std::unique_ptr<ExpressionNode> ParsePrimaryExpression();

    public:
     Parser(std::string text);
     std::unique_ptr<SyntaxTree> Parse();
     //const std::vector<std::string>& Errors() const { return mVecErrors; }
};