// Copyright (c) 2022 F. Lotfi All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include <sstream>
// #include<algorithm> // for copy() and assign()
// #include<iterator> // for back_inserter
#include "AssignmentExpressionNode.h"
#include "BinaryExpressionNode.h"
#include "IdentifierExpressionNode.h"
#include "Lexer.h"
#include "LiteralExpressionNode.h"
#include "ParenthesizedExpressionNode.h"
#include "Parser.h"
#include "UnaryExpressionNode.h"

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
  mRecords.ReportUnexpectedToken(Current()->Span().Start(),
                                 Current()->Span().End(), Current()->Type(),
                                 type);
  return std::make_shared<SyntaxToken>(type, Current()->Span().Start(),
                                       Current()->Span().End(), "");
}

Parser::Parser(std::string text) : mTokens(), mPosition(0), mRecords("Parser") {
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

  mRecords.assign(lex.Errors().begin(), lex.Errors().end());
}

std::unique_ptr<ExpressionNode>
Parser::ParseBinaryExpression(int parentPrecedence) {
  std::unique_ptr<ExpressionNode> left = nullptr;
  int unaryOperatorPrecedence =
      SyntaxOrder::GetUnaryOperatorPrecedence(Current()->Type());
  if (unaryOperatorPrecedence != 0 &&
      unaryOperatorPrecedence >= parentPrecedence) {
    auto operatorToken = Next();
    std::unique_ptr<ExpressionNode> operand =
        ParseBinaryExpression(unaryOperatorPrecedence);
    left = std::make_unique<UnaryExpressionNode>(operatorToken,
                                                 std::move(operand));
  } else {
    left = ParsePrimaryExpression();
  }
  while (true) {
    int precedence =
        SyntaxOrder::GetBinaryOperatorPrecedence(Current()->Type());
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
  if (Current()->Type() == SyntaxType::TrueKeyword ||
      Current()->Type() == SyntaxType::FalseKeyword) {
    auto boolToken = Match(Current()->Type());
    return std::make_unique<LiteralExpressionNode>(boolToken);
  }
  if (Current()->Type() == SyntaxType::IdentifierToken) {
    auto identifierToken = Match(Current()->Type());
    return std::make_unique<IdentifierExpressionNode>(identifierToken);
  }

  auto numberToken = Match(SyntaxType::NumberToken);
  return std::make_unique<LiteralExpressionNode>(numberToken);
}

std::unique_ptr<ExpressionNode> Parser::ParseAssignmentExpression() {
  if (Peek(0)->Type() != SyntaxType::IdentifierToken) {
    return ParseBinaryExpression();
  }
  switch (Peek(1)->Type().GetValue()) {
  case SyntaxType::PlusEqualsToken:
  case SyntaxType::MinusEqualsToken:
  case SyntaxType::StarEqualsToken:
  case SyntaxType::SlashEqualsToken:
  case SyntaxType::AmpersandEqualsToken:
  case SyntaxType::PipeEqualsToken:
  case SyntaxType::HatEqualsToken:
  case SyntaxType::EqualsToken: {
    auto identifierToken = Next();
    auto operatorToken = Next();
    auto right = ParseAssignmentExpression();
    return std::make_unique<AssignmentExpressionNode>(
        identifierToken, operatorToken, std::move(right));
  }
  default:
    break;
  }
  return ParseBinaryExpression();
}

std::unique_ptr<SyntaxTree> Parser::Parse() {

  auto expression = ParseAssignmentExpression();
  return std::make_unique<SyntaxTree>(mRecords, std::move(expression));
}