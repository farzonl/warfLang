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

std::shared_ptr<SyntaxToken> Parser::Match(SyntaxKind kind) {
  if (Current()->Kind() == kind) {
    return Next();
  }
  mRecords.ReportUnexpectedToken(Current()->Span().Start(),
                                 Current()->Span().End(), Current()->Kind(),
                                 kind);
  return std::make_shared<SyntaxToken>(kind, Current()->Span().Start(),
                                       Current()->Span().End(), "");
}

Parser::Parser(std::string text) : mTokens(), mPosition(0), mRecords("Parser") {
  Lexer lex(text);
  SyntaxToken *pCurrToken = nullptr;
  std::unique_ptr<SyntaxToken> upCurrToken = nullptr;
  do {
    upCurrToken = lex.NextToken();
    if (upCurrToken->Kind() != SyntaxKind::WhiteSpaceToken &&
        upCurrToken->Kind() != SyntaxKind::UnknownToken) {
      mTokens.push_back(std::move(upCurrToken));
      pCurrToken = mTokens[mTokens.size() - 1].get();
    } else {
      pCurrToken = upCurrToken.get();
    }

  } while (pCurrToken && pCurrToken->Kind() != SyntaxKind::EndOfFileToken);

  mRecords.assign(lex.Errors().begin(), lex.Errors().end());
}

std::unique_ptr<ExpressionNode>
Parser::ParseBinaryExpression(int parentPrecedence) {
  std::unique_ptr<ExpressionNode> left = nullptr;
  int unaryOperatorPrecedence =
      SyntaxOrder::GetUnaryOperatorPrecedence(Current()->Kind());
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
        SyntaxOrder::GetBinaryOperatorPrecedence(Current()->Kind());
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
  if (Current()->Kind() == SyntaxKind::OpenParenthesisToken) {
    auto left = Next();
    auto expression = ParseBinaryExpression();

    auto right = Match(SyntaxKind::CloseParenthesisToken);
    return std::make_unique<ParenthesizedExpressionNode>(
        left, std::move(expression), right);
  }
  if (Current()->Kind() == SyntaxKind::TrueKeyword ||
      Current()->Kind() == SyntaxKind::FalseKeyword) {
    auto boolToken = Match(Current()->Kind());
    return std::make_unique<LiteralExpressionNode>(boolToken);
  }
  if (Current()->Kind() == SyntaxKind::IdentifierToken) {
    auto identifierToken = Match(Current()->Kind());
    return std::make_unique<IdentifierExpressionNode>(identifierToken);
  }

  auto numberToken = Match(SyntaxKind::NumberToken);
  return std::make_unique<LiteralExpressionNode>(numberToken);
}

std::unique_ptr<ExpressionNode> Parser::ParseAssignmentExpression() {
  if (Peek(0)->Kind() != SyntaxKind::IdentifierToken) {
    return ParseBinaryExpression();
  }
  switch (Peek(1)->Kind().GetValue()) {
  case SyntaxKind::PlusEqualsToken:
  case SyntaxKind::MinusEqualsToken:
  case SyntaxKind::StarEqualsToken:
  case SyntaxKind::SlashEqualsToken:
  case SyntaxKind::AmpersandEqualsToken:
  case SyntaxKind::PipeEqualsToken:
  case SyntaxKind::HatEqualsToken:
  case SyntaxKind::EqualsToken: {
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