// Copyright (c) 2022 F. Lotfi All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include <limits>
#include <sstream>
// #include<algorithm> // for copy() and assign()
// #include<iterator> // for back_inserter
#include "AssignmentExpressionNode.h"
#include "BinaryExpressionNode.h"
#include "BreakStatementSyntaxNode.h"
#include "CallExpressionNode.h"
#include "ContinueStatementSyntaxNode.h"
#include "DoWhileStatementSyntaxNode.h"
#include "ForStatementSyntaxNode.h"
#include "FunctionDeclarationSyntaxNode.h"
#include "IdentifierExpressionNode.h"
#include "IfStatementSyntaxNode.h"
#include "Lexer.h"
#include "LiteralExpressionNode.h"
#include "ParenthesizedExpressionNode.h"
#include "Parser.h"
#include "ReturnStatementSyntaxNode.h"
#include "UnaryExpressionNode.h"
#include "WhileStatementSyntaxNode.h"

static Value::Type ParseTypeAnnotation(const std::string &text) {
  if (text == "int" || text == "number") {
    return Value::Type::Number;
  }
  if (text == "bool" || text == "boolean") {
    return Value::Type::Boolean;
  }
  if (text == "string" || text == "str") {
    return Value::Type::String;
  }
  return Value::Type::Unknown;
}

namespace {
struct ParseDepthGuard {
  int32_t &depth;
  explicit ParseDepthGuard(int32_t &d) : depth(d) { ++depth; }
  ~ParseDepthGuard() { --depth; }
};
} // namespace

std::shared_ptr<SyntaxToken> Parser::Peek(int32_t offset) {
  int32_t index = mPosition + offset;
  if (index >= static_cast<int32_t>(mTokens.size())) {
    return mTokens.back();
  }
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
  // other unexpected token, advance so recovery keeps making forward progress
  // and does not spin forever allocating AST nodes.
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
  if (mDepth >= kMaxParseDepth) {
    auto current = Current();
    mRecords.ReportUnexpectedToken(current->Span().Start(),
                                   current->Span().End(), current->Kind(),
                                   SyntaxKind::NumberToken);
    if (current->Kind() != SyntaxKind::EndOfFileToken) {
      Next();
    }
    auto tok = std::make_shared<SyntaxToken>(SyntaxKind::NumberToken,
                                             current->Span().Start(),
                                             current->Span().End(), "");
    return std::make_unique<LiteralExpressionNode>(tok);
  }
  ParseDepthGuard depthGuard(mDepth);
  std::unique_ptr<ExpressionNode> left = nullptr;
  int unaryOperatorPrecedence =
      SyntaxOrder::GetUnaryOperatorPrecedence(Current()->Kind());
  if (unaryOperatorPrecedence != 0 &&
      unaryOperatorPrecedence >= parentPrecedence) {
    auto operatorToken = Next();
    std::unique_ptr<ExpressionNode> operand =
        ParseBinaryExpression(unaryOperatorPrecedence);
    if (operatorToken->Kind() == SyntaxKind::MinusToken) {
      if (auto lit = dynamic_cast<LiteralExpressionNode *>(operand.get())) {
        if (lit->LiteralToken()->Kind() == SyntaxKind::NumberToken &&
            lit->LiteralToken()->GetValue().isInt() &&
            lit->LiteralToken()->GetValue().asInt() ==
                std::numeric_limits<int32_t>::min()) {
          mRecords.RemoveNumericOverflow(lit->LiteralToken()->Span());
        }
      }
    }
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
  if (Current()->Kind() == SyntaxKind::IdentifierToken &&
      Peek(1)->Kind() == SyntaxKind::OpenParenthesisToken) {
    return ParseCallExpression();
  }
  if (Current()->Kind() == SyntaxKind::IdentifierToken) {
    auto identifierToken = Match(Current()->Kind());
    return std::make_unique<IdentifierExpressionNode>(identifierToken);
  }
  if (Current()->Kind() == SyntaxKind::StringToken) {
    auto stringToken = Match(SyntaxKind::StringToken);
    return std::make_unique<LiteralExpressionNode>(stringToken);
  }

  auto numberToken = Match(SyntaxKind::NumberToken);
  return std::make_unique<LiteralExpressionNode>(numberToken);
}

std::unique_ptr<ExpressionNode> Parser::ParseCallExpression() {
  auto identifier = Match(SyntaxKind::IdentifierToken);
  auto openParenthesis = Match(SyntaxKind::OpenParenthesisToken);
  auto arguments = std::vector<std::unique_ptr<ExpressionNode>>();
  auto commas = std::vector<std::shared_ptr<SyntaxToken>>();
  while (Current()->Kind() != SyntaxKind::CloseParenthesisToken &&
         Current()->Kind() != SyntaxKind::EndOfFileToken) {
    arguments.push_back(ParseAssignmentExpression());
    if (Current()->Kind() != SyntaxKind::CloseParenthesisToken) {
      commas.push_back(Match(SyntaxKind::CommaToken));
    }
  }
  auto closeParenthesis = Match(SyntaxKind::CloseParenthesisToken);
  return std::make_unique<CallExpressionNode>(
      identifier, openParenthesis, std::move(arguments), std::move(commas),
      closeParenthesis);
}

std::unique_ptr<ExpressionNode> Parser::ParseAssignmentExpression() {
  if (mDepth >= kMaxParseDepth) {
    auto current = Current();
    mRecords.ReportUnexpectedToken(current->Span().Start(),
                                   current->Span().End(), current->Kind(),
                                   SyntaxKind::NumberToken);
    if (current->Kind() != SyntaxKind::EndOfFileToken) {
      Next();
    }
    auto tok = std::make_shared<SyntaxToken>(SyntaxKind::NumberToken,
                                             current->Span().Start(),
                                             current->Span().End(), "");
    return std::make_unique<LiteralExpressionNode>(tok);
  }
  ParseDepthGuard depthGuard(mDepth);
  if (Peek(0)->Kind() != SyntaxKind::IdentifierToken) {
    return ParseBinaryExpression();
  }
  switch (Peek(1)->Kind().GetValue()) {
  case SyntaxKind::PlusEqualsToken:
  case SyntaxKind::MinusEqualsToken:
  case SyntaxKind::StarEqualsToken:
  case SyntaxKind::SlashEqualsToken:
  case SyntaxKind::PercentEqualsToken:
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
    while (Current()->Kind() == SyntaxKind::SemicolonToken) {
      Next();
    }
    if (Current()->Kind() == SyntaxKind::EndOfFileToken) {
      break;
    }
    statements.push_back(ParseStatement());
  }
  auto endOfFileToken = Match(SyntaxKind::EndOfFileToken);
  return std::make_unique<CompilationUnitSyntaxNode>(std::move(statements),
                                                     endOfFileToken);
}

std::unique_ptr<StatementSyntaxNode> Parser::ParseStatement() {
  if (mDepth >= kMaxParseDepth) {
    auto current = Current();
    mRecords.ReportUnexpectedToken(current->Span().Start(),
                                   current->Span().End(), current->Kind(),
                                   SyntaxKind::SemicolonToken);
    if (current->Kind() != SyntaxKind::EndOfFileToken) {
      Next();
    }
    auto tok = std::make_shared<SyntaxToken>(SyntaxKind::NumberToken,
                                             current->Span().Start(),
                                             current->Span().End(), "");
    return std::make_unique<ExpressionStatementSyntaxNode>(
        std::make_unique<LiteralExpressionNode>(tok));
  }
  ParseDepthGuard depthGuard(mDepth);
  std::unique_ptr<StatementSyntaxNode> statement;
  switch (Current()->Kind().GetValue()) {
  case SyntaxKind::OpenBraceToken:
    statement = ParseBlockStatement();
    break;
  case SyntaxKind::LetKeyword:
  case SyntaxKind::VarKeyword:
    statement = ParseVariableDeclaration();
    break;
  case SyntaxKind::IfKeyword:
    statement = ParseIfStatement();
    break;
  case SyntaxKind::WhileKeyword:
    statement = ParseWhileStatement();
    break;
  case SyntaxKind::DoKeyword:
    statement = ParseDoWhileStatement();
    break;
  case SyntaxKind::ForKeyword:
    statement = ParseForStatement();
    break;
  case SyntaxKind::BreakKeyword:
    statement = ParseBreakStatement();
    break;
  case SyntaxKind::ContinueKeyword:
    statement = ParseContinueStatement();
    break;
  case SyntaxKind::ReturnKeyword:
    statement = ParseReturnStatement();
    break;
  case SyntaxKind::FunctionKeyword:
    statement = ParseFunctionDeclaration();
    break;
  default:
    statement = ParseExpressionStatement();
    break;
  }
  while (Current()->Kind() == SyntaxKind::SemicolonToken) {
    Next();
  }
  return statement;
}

std::unique_ptr<StatementSyntaxNode> Parser::ParseBreakStatement() {
  auto breakKeyword = Match(SyntaxKind::BreakKeyword);
  return std::make_unique<BreakStatementSyntaxNode>(breakKeyword);
}

std::unique_ptr<StatementSyntaxNode> Parser::ParseContinueStatement() {
  auto continueKeyword = Match(SyntaxKind::ContinueKeyword);
  return std::make_unique<ContinueStatementSyntaxNode>(continueKeyword);
}

std::unique_ptr<StatementSyntaxNode> Parser::ParseReturnStatement() {
  auto returnKeyword = Match(SyntaxKind::ReturnKeyword);
  std::unique_ptr<ExpressionNode> expression;
  if (Current()->Kind() != SyntaxKind::CloseBraceToken &&
      Current()->Kind() != SyntaxKind::EndOfFileToken &&
      Current()->Kind() != SyntaxKind::SemicolonToken) {
    expression = ParseAssignmentExpression();
  }
  return std::make_unique<ReturnStatementSyntaxNode>(returnKeyword,
                                                     std::move(expression));
}

std::unique_ptr<StatementSyntaxNode> Parser::ParseFunctionDeclaration() {
  auto functionKeyword = Match(SyntaxKind::FunctionKeyword);
  auto identifier = Match(SyntaxKind::IdentifierToken);
  auto openParenthesis = Match(SyntaxKind::OpenParenthesisToken);
  auto parameters = std::vector<std::shared_ptr<SyntaxToken>>();
  auto parameterTypes = std::vector<Value::Type>();
  auto commas = std::vector<std::shared_ptr<SyntaxToken>>();
  while (Current()->Kind() != SyntaxKind::CloseParenthesisToken &&
         Current()->Kind() != SyntaxKind::EndOfFileToken) {
    parameters.push_back(Match(SyntaxKind::IdentifierToken));
    if (Current()->Kind() == SyntaxKind::ColonToken) {
      Next();
      auto typeToken = Match(SyntaxKind::IdentifierToken);
      parameterTypes.push_back(ParseTypeAnnotation(typeToken->Text()));
    } else {
      parameterTypes.push_back(Value::Type::Unknown);
    }
    if (Current()->Kind() != SyntaxKind::CloseParenthesisToken) {
      commas.push_back(Match(SyntaxKind::CommaToken));
    }
  }
  auto closeParenthesis = Match(SyntaxKind::CloseParenthesisToken);
  auto returnType = Value::Type::Unknown;
  if (Current()->Kind() == SyntaxKind::ColonToken) {
    Next();
    auto returnTypeToken = Match(SyntaxKind::IdentifierToken);
    returnType = ParseTypeAnnotation(returnTypeToken->Text());
  }
  auto body = ParseBlockStatement();
  return std::make_unique<FunctionDeclarationSyntaxNode>(
      functionKeyword, identifier, openParenthesis, std::move(parameters),
      std::move(commas), closeParenthesis, std::move(body),
      std::move(parameterTypes), returnType);
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

std::unique_ptr<StatementSyntaxNode> Parser::ParseDoWhileStatement() {
  auto doKeyword = Match(SyntaxKind::DoKeyword);
  auto body = ParseStatement();
  auto whileKeyword = Match(SyntaxKind::WhileKeyword);
  auto openParenthesis = Match(SyntaxKind::OpenParenthesisToken);
  auto condition = ParseAssignmentExpression();
  auto closeParenthesis = Match(SyntaxKind::CloseParenthesisToken);
  return std::make_unique<DoWhileStatementSyntaxNode>(
      doKeyword, std::move(body), whileKeyword, openParenthesis,
      std::move(condition), closeParenthesis);
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
    while (Current()->Kind() == SyntaxKind::SemicolonToken) {
      Next();
    }
    if (Current()->Kind() == SyntaxKind::EndOfFileToken ||
        Current()->Kind() == SyntaxKind::CloseBraceToken) {
      break;
    }
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
  Value::Type typeAnnotation = Value::Type::Unknown;
  if (Current()->Kind() == SyntaxKind::ColonToken) {
    Next();
    auto typeToken = Match(SyntaxKind::IdentifierToken);
    typeAnnotation = ParseTypeAnnotation(typeToken->Text());
  }
  auto equals = Match(SyntaxKind::EqualsToken);
  auto initializer = ParseAssignmentExpression();
  return std::make_unique<VariableDeclarationSyntaxNode>(
      keyword, identifier, equals, std::move(initializer), typeAnnotation);
}

std::unique_ptr<ExpressionStatementSyntaxNode>
Parser::ParseExpressionStatement() {
  auto expression = ParseAssignmentExpression();
  return std::make_unique<ExpressionStatementSyntaxNode>(std::move(expression));
}
