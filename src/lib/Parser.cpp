#include <sstream>
//#include<algorithm> // for copy() and assign()
//#include<iterator> // for back_inserter
#include "Parser.h"
#include "Lexer.h"
#include "BinaryExpressionNode.h"
#include "ParenthesizedExpressionNode.h"
#include "NumberNode.h"


std::shared_ptr <SyntaxToken> Parser::Peek(int32_t offset) {
    int32_t index = mPosition + offset;
    //if (index >= mTokens.size()) {
    //    return nullptr;
    //}
    assert(index < mTokens.size());
    return mTokens[index];
}

std::shared_ptr <SyntaxToken> Parser::Current() {
    return Peek(0);
}

std::shared_ptr <SyntaxToken> Parser::Next() {
    auto curr = Current();
    mPosition++;
    return curr;
}

std::shared_ptr<SyntaxToken> Parser::Match(SyntaxType type) {
    if (Current()->Type() == type) {
        return Next();
    }
    std::stringstream ss;
    ss << "ParseError: Unexpected token: < " << Current()->Type() 
       << " > Expected: < " << type << " >";
    mVecErrors.push_back(ss.str());
    return std::make_shared<SyntaxToken>(type, Current()->Position(), "");
}

Parser::Parser(std::string text) : mTokens(), mPosition(0), mVecErrors(){
    Lexer lex(text);
    SyntaxToken* pCurrToken = nullptr;
    std::unique_ptr<SyntaxToken> upCurrToken = nullptr;
    do {
        upCurrToken = lex.NextToken();
        if (upCurrToken->Type() != SyntaxType::WhiteSpaceToken &&
            upCurrToken->Type() != SyntaxType::UnknownToken)
        {
            mTokens.push_back(std::move(upCurrToken));
            pCurrToken = mTokens[mTokens.size() - 1].get();
        } else {
            pCurrToken = upCurrToken.get();
        }

    } while (pCurrToken && pCurrToken->Type() != SyntaxType::EndOfFileToken);

    mVecErrors.assign(lex.Errors().begin(), lex.Errors().end());
    //copy(lex.Errors().begin(), lex.Errors().end(), back_inserter(mVecErrors));
}

std::unique_ptr<ExpressionNode> Parser::ParseTerm() {
    //if (Current() == nullptr) {
    //    return nullptr;
    //}

    std::unique_ptr<ExpressionNode> left = ParseFactor();
    while (Current()->Type() == SyntaxType::PlusToken ||
        Current()->Type()    == SyntaxType::MinusToken) {
        auto operatorToken = Next();
        std::unique_ptr<ExpressionNode> right = ParseFactor();
        left = std::make_unique<BinaryExpressionNode>(std::move(left), operatorToken, std::move(right));
    }

    return std::move(left);
}


std::unique_ptr<ExpressionNode> Parser::ParseFactor() {
    //if (Current() == nullptr) {
    //    return nullptr;
    //}

    std::unique_ptr<ExpressionNode> left = ParsePrimaryExpression();
    while (Current()->Type() == SyntaxType::StarToken ||
        Current()->Type() == SyntaxType::SlashToken) {
        auto operatorToken = Next();
        std::unique_ptr<ExpressionNode> right = ParsePrimaryExpression();
        left = std::make_unique<BinaryExpressionNode>(std::move(left), operatorToken, std::move(right));
    }

    return std::move(left);
}

std::unique_ptr<ExpressionNode> Parser::ParsePrimaryExpression() {
    //if (Current() == nullptr) {
    //    return nullptr;
    //}

    if (Current()->Type() == SyntaxType::OpenParenthesisToken) {
        auto left = Next();
        auto expression = ParseTerm();

        auto right = Match(SyntaxType::CloseParenthesisToken);
        return  std::make_unique <ParenthesizedExpressionNode>(left, std::move(expression), right);
    }

    auto numberToken = Match(SyntaxType::NumberToken);
    return std::make_unique <NumberNode>(numberToken);
}

std::unique_ptr<SyntaxTree> Parser::Parse() {
    
    auto expression = ParseTerm();
    //if (expression == nullptr) {
    //    return std::make_unique<ExpressionNode>(SyntaxType::UnknownToken);
    //}
    return  std::make_unique<SyntaxTree>(mVecErrors, std::move(expression));
}