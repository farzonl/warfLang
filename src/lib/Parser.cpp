#include <sstream>
//#include<algorithm> // for copy() and assign()
//#include<iterator> // for back_inserter
#include "BinaryExpressionNode.h"
#include "UnaryExpressionNode.h"
#include "Lexer.h"
#include "NumberNode.h"
#include "ParenthesizedExpressionNode.h"
#include "Parser.h"

std::shared_ptr<SyntaxToken> Parser::Peek(int32_t offset) {
  int32_t index = mPosition + offset;
  assert(index < mTokens.size());
  return mTokens[index];
}

std::shared_ptr<SyntaxToken> Parser::Current() { return Peek(0); }

std::shared_ptr<SyntaxToken> Parser::Next() {
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

Parser::Parser(std::string text) : mTokens(), mPosition(0), mVecErrors() {
  Lexer lex(text);
  SyntaxToken *pCurrToken = nullptr;
  std::unique_ptr<SyntaxToken> upCurrToken = nullptr;
  do {
    upCurrToken = lex.NextToken();
    if (upCurrToken->Type() != SyntaxType::WhiteSpaceToken &&
        upCurrToken->Type() != SyntaxType::UnknownToken) {
      mTokens.push_back(std::move(upCurrToken));
      pCurrToken = mTokens[mTokens.size() - 1].get();
    } else {
      pCurrToken = upCurrToken.get();
    }

  } while (pCurrToken && pCurrToken->Type() != SyntaxType::EndOfFileToken);

  mVecErrors.assign(lex.Errors().begin(), lex.Errors().end());
}

std::unique_ptr<ExpressionNode> Parser::ParseBinaryExpression(int parentPrecedence) {
  std::unique_ptr<ExpressionNode> left = nullptr;
  int unaryOperatorPrecedence = SyntaxOrder::GetUnaryOperatorPrecedence(Current()->Type());
  if (unaryOperatorPrecedence != 0 && unaryOperatorPrecedence >= parentPrecedence) {
      auto operatorToken = Next();
      std::unique_ptr<ExpressionNode> operand = ParseBinaryExpression(unaryOperatorPrecedence);
      left = std::make_unique<UnaryExpressionNode>(operatorToken, std::move(operand));
  } else {
    left = ParsePrimaryExpression();
  }
  while (true) {
    int precedence = SyntaxOrder::GetBinaryOperatorPrecedence(Current()->Type());
    if (precedence == 0 || precedence <= parentPrecedence) {
        break;
    }
    auto operatorToken = Next();
    std::unique_ptr<ExpressionNode> right = ParseBinaryExpression(precedence);
    left = std::make_unique<BinaryExpressionNode>(
        std::move(left), operatorToken, std::move(right));
  }
  return std::move(left);
}

std::unique_ptr<ExpressionNode> Parser::ParsePrimaryExpression() {
  if (Current()->Type() == SyntaxType::OpenParenthesisToken) {
    auto left = Next();
    auto expression = ParseBinaryExpression();

    auto right = Match(SyntaxType::CloseParenthesisToken);
    return std::make_unique<ParenthesizedExpressionNode>(
        left, std::move(expression), right);
  }

  auto numberToken = Match(SyntaxType::NumberToken);
  return std::make_unique<NumberNode>(numberToken);
}

std::unique_ptr<SyntaxTree> Parser::Parse() {

  auto expression = ParseBinaryExpression();
  // if (expression == nullptr) {
  //    return std::make_unique<ExpressionNode>(SyntaxType::UnknownToken);
  //}
  return std::make_unique<SyntaxTree>(mVecErrors, std::move(expression));
}