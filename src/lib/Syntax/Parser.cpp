// Copyright (c) 2022 F. Lotfi All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include <sstream>
// #include<algorithm> // for copy() and assign()
// #include<iterator> // for back_inserter
#include "AssignmentExpressionNode.h"
#include "BinaryExpressionNode.h"
#include "ForStatementSyntaxNode.h"
#include "IdentifierExpressionNode.h"
#include "IfStatementSyntaxNode.h"
#include "Lexer.h"
#include "LiteralExpressionNode.h"
#include "ParenthesizedExpressionNode.h"
#include "Parser.h"
#include "UnaryExpressionNode.h"
#include "WhileStatementSyntaxNode.h"

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
  auto current = Current();
  if (current->Kind() == kind) {
    return Next();
  }
  mRecords.ReportUnexpectedToken(current->Span().Start(), current->Span().End(),
                                 current->Kind(), kind);

  // A mismatched EOF is a terminal condition, not a token to consume. For any
  // other unexpected token, advance so recovery keeps making forward progress and
  // does not spin forever allocating AST nodes.
  if (current->Kind() != SyntaxKind::EndOfFileToken) {
    mPosition++;
  }
  return std::make_shared<SyntaxToken>(kind, current->Span().Start(),
                                       current->Span().End(), "");
}

Parser::Parser(std::string text) : mTokens(), mPosition(0), mRecords("Parser") {
  Lexer lex(text);
  SyntaxToken *pCurrToken = nullptr;
  std::unique_ptr<SyntaxToken> upCurrToken = nullptr;
  do {
    upCurrToken = lex.NextToken();
    if (upCurrToken->Kind() != SyntaxKind::WhiteSpaceToken &&
        upCurrToken->Kind() != SyntaxKind::CommentToken &&
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

std::unique_ptr<CompilationUnitSyntaxNode> Parser::ParseCompilationUnit() {
  auto statements = std::vector<std::unique_ptr<StatementSyntaxNode>>();
  while (Current()->Kind() != SyntaxKind::EndOfFileToken) {
    statements.push_back(ParseStatement());
  }
  auto endOfFileToken = Match(SyntaxKind::EndOfFileToken);
  return std::make_unique<CompilationUnitSyntaxNode>(std::move(statements),
                                                     endOfFileToken);
}

std::unique_ptr<StatementSyntaxNode> Parser::ParseStatement() {
  switch (Current()->Kind().GetValue()) {
  case SyntaxKind::OpenBraceToken:
    return ParseBlockStatement();
  case SyntaxKind::LetKeyword:
  case SyntaxKind::VarKeyword:
    return ParseVariableDeclaration();
  case SyntaxKind::IfKeyword:
    return ParseIfStatement();
  case SyntaxKind::WhileKeyword:
    return ParseWhileStatement();
  case SyntaxKind::ForKeyword:
    return ParseForStatement();
  default:
    return ParseExpressionStatement();
  }
}

std::unique_ptr<StatementSyntaxNode> Parser::ParseIfStatement() {
  auto ifKeyword = Match(SyntaxKind::IfKeyword);
  auto openParenthesis = Match(SyntaxKind::OpenParenthesisToken);
  auto condition = ParseAssignmentExpression();
  auto closeParenthesis = Match(SyntaxKind::CloseParenthesisToken);
  auto thenStatement = ParseStatement();
  std::shared_ptr<SyntaxToken> elseKeyword;
  std::unique_ptr<StatementSyntaxNode> elseStatement;
  if (Current()->Kind() == SyntaxKind::ElseKeyword) {
    elseKeyword = Match(SyntaxKind::ElseKeyword);
    elseStatement = ParseStatement();
  }
  return std::make_unique<IfStatementSyntaxNode>(
      ifKeyword, openParenthesis, std::move(condition), closeParenthesis,
      std::move(thenStatement), elseKeyword, std::move(elseStatement));
}

std::unique_ptr<StatementSyntaxNode> Parser::ParseWhileStatement() {
  auto whileKeyword = Match(SyntaxKind::WhileKeyword);
  auto openParenthesis = Match(SyntaxKind::OpenParenthesisToken);
  auto condition = ParseAssignmentExpression();
  auto closeParenthesis = Match(SyntaxKind::CloseParenthesisToken);
  auto body = ParseStatement();
  return std::make_unique<WhileStatementSyntaxNode>(
      whileKeyword, openParenthesis, std::move(condition), closeParenthesis,
      std::move(body));
}

std::unique_ptr<StatementSyntaxNode> Parser::ParseForStatement() {
  auto forKeyword = Match(SyntaxKind::ForKeyword);
  auto openParenthesis = Match(SyntaxKind::OpenParenthesisToken);
  std::unique_ptr<StatementSyntaxNode> initializer;
  if (Current()->Kind() != SyntaxKind::SemicolonToken) {
    initializer = Current()->Kind() == SyntaxKind::LetKeyword ||
                          Current()->Kind() == SyntaxKind::VarKeyword
                      ? ParseVariableDeclaration()
                      : ParseExpressionStatement();
  }
  auto firstSemicolon = Match(SyntaxKind::SemicolonToken);
  auto condition = ParseAssignmentExpression();
  auto secondSemicolon = Match(SyntaxKind::SemicolonToken);
  auto increment = ParseAssignmentExpression();
  auto closeParenthesis = Match(SyntaxKind::CloseParenthesisToken);
  auto body = ParseStatement();
  return std::make_unique<ForStatementSyntaxNode>(
      forKeyword, openParenthesis, std::move(initializer), firstSemicolon,
      std::move(condition), secondSemicolon, std::move(increment),
      closeParenthesis, std::move(body));
}

std::unique_ptr<BlockStatementSyntaxNode> Parser::ParseBlockStatement() {
  auto statements = std::vector<std::unique_ptr<StatementSyntaxNode>>();
  auto openBraceToken = Match(SyntaxKind::OpenBraceToken);
  while (Current()->Kind() != SyntaxKind::EndOfFileToken &&
         Current()->Kind() != SyntaxKind::CloseBraceToken) {
    auto statement = ParseStatement();
    statements.push_back(std::move(statement));
  }
  auto closeBraceToken = Match(SyntaxKind::CloseBraceToken);
  return std::make_unique<BlockStatementSyntaxNode>(
      openBraceToken, std::move(statements), closeBraceToken);
}

std::unique_ptr<StatementSyntaxNode> Parser::ParseVariableDeclaration() {
  auto expected = Current()->Kind() == SyntaxKind::LetKeyword
                      ? SyntaxKind::LetKeyword
                      : SyntaxKind::VarKeyword;
  auto keyword = Match(expected);
  auto identifier = Match(SyntaxKind::IdentifierToken);
  auto equals = Match(SyntaxKind::EqualsToken);
  auto initializer = ParseAssignmentExpression();
  return std::make_unique<VariableDeclarationSyntaxNode>(
      keyword, identifier, equals, std::move(initializer));
}

std::unique_ptr<ExpressionStatementSyntaxNode>
Parser::ParseExpressionStatement() {
  auto expression = ParseAssignmentExpression();
  return std::make_unique<ExpressionStatementSyntaxNode>(std::move(expression));
}
