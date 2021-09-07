#pragma once
#include <string>
#include <vector>
#include <memory>
#include "SyntaxToken.h"




class Lexer {
    private:
        std::string mText;
        int32_t mPosition;
        ValueType mValue;
        std::vector<std::string> mVecErrors;
        void ReadToken(SyntaxType &type);
        void ParseNumber();
    public:
        Lexer(std::string text);
        char CurrentToken();
        std::unique_ptr<SyntaxToken> NextToken();
        const std::vector<std::string>& Errors() const { return mVecErrors; }
};