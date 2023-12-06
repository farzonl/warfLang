/*
 * Copyright (c) 2022 F. Lotfi All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#pragma once
#include <cassert>
#include <iostream>
#include <stdint.h>
#include <string>
#include <unordered_map>

class SyntaxKind {
public:
  enum Value : uint8_t {
    UnknownToken,
    EndOfFileToken,
    WhiteSpaceToken,
    PlusToken,
    PlusEqualsToken,
    MinusToken,
    MinusEqualsToken,
    StarToken,
    StarEqualsToken,
    SlashToken,
    SlashEqualsToken,
    OpenParenthesisToken,
    CloseParenthesisToken,
    OpenBraceToken,
    CloseBraceToken,
    OpenBracketToken,
    CloseBracketToken,
    ColonToken,
    CommaToken,
    TildeToken,
    HatToken,
    HatEqualsToken,
    AmpersandAmpersandToken,
    AmpersandEqualsToken,
    AmpersandToken,
    PipePipeToken,
    PipeEqualsToken,
    PipeToken,
    EqualsToken,
    EqualsEqualsToken,
    BangToken,
    BangEqualsToken,
    LessToken,
    LessOrEqualsToken,
    GreaterToken,
    GreaterOrEqualsToken,
    NumberToken,
    // Not Real Tokens
    IdentifierToken,
    IdentifierExpression,
    UnaryExpression,
    BinaryExpression,
    ParenthesizedExpression,
    LiteralExpression,
    AssignmentExpression,
    // Keywords
    BreakKeyword,
    ContinueKeyword,
    IfKeyword,
    ElseKeyword,
    TrueKeyword,
    FalseKeyword,
    ForKeyword,
    WhileKeyword,
    DoKeyword,
    FunctionKeyword,
    ReturnKeyword,
    LetKeyword,
    VarKeyword,
  };

  SyntaxKind() : value(UnknownToken) {}
  constexpr SyntaxKind(Value aSyntaxKind) : value(aSyntaxKind) {}

  // Allow switch and comparisons.
  Value GetValue() const { return value; }
  // Note: I want to do this but it conflicts with operator ==
  // constexpr operator Value() const { return value; }

  // Prevent usage: if(SyntaxKind)
  explicit operator bool() const = delete;
  constexpr bool operator==(SyntaxKind a) const { return value == a.value; }
  constexpr bool operator!=(SyntaxKind a) const { return value != a.value; }

  static SyntaxKind GetKeywordKind(std::string &text);

  struct Hash {
    size_t operator()(const SyntaxKind &a) const {
      return static_cast<size_t>(a.value);
    }
  };

private:
  Value value;
};

static const std::unordered_map<bool, std::string> boolToNameMap = {
    {true, "true"},
    {false, "false"},
};

static const std::unordered_map<std::string, bool> boolStrToValueMap = {
    {"true", true},
    {"false", false},
};

struct SyntaxOrder {
  static int GetUnaryOperatorPrecedence(SyntaxKind type);
  static int GetBinaryOperatorPrecedence(SyntaxKind type);

private:
  SyntaxOrder() = delete;
};

static const std::unordered_map<std::string, SyntaxKind> KeywordMap = {
    {"break", SyntaxKind::BreakKeyword},
    {"continue", SyntaxKind::ContinueKeyword},
    {"if", SyntaxKind::IfKeyword},
    {"else", SyntaxKind::ElseKeyword},
    {"true", SyntaxKind::TrueKeyword},
    {"false", SyntaxKind::FalseKeyword},
    {"for", SyntaxKind::ForKeyword},
    {"while", SyntaxKind::WhileKeyword},
    {"do", SyntaxKind::DoKeyword},
    {"function", SyntaxKind::FunctionKeyword},
    {"return", SyntaxKind::ReturnKeyword},
    {"let", SyntaxKind::LetKeyword},
    {"var", SyntaxKind::VarKeyword},
};

static const std::unordered_map<SyntaxKind, std::string, SyntaxKind::Hash>
    KeywordToStrMap = {
        {SyntaxKind::BreakKeyword, "break"},
        {SyntaxKind::ContinueKeyword, "continue"},
        {SyntaxKind::IfKeyword, "if"},
        {SyntaxKind::ElseKeyword, "else"},
        {SyntaxKind::TrueKeyword, "true"},
        {SyntaxKind::FalseKeyword, "false"},
        {SyntaxKind::ForKeyword, "for"},
        {SyntaxKind::WhileKeyword, "while"},
        {SyntaxKind::DoKeyword, "do"},
        {SyntaxKind::FunctionKeyword, "function"},
        {SyntaxKind::ReturnKeyword, "return"},
        {SyntaxKind::LetKeyword, "let"},
        {SyntaxKind::VarKeyword, "var"},
};

static const std::unordered_map<std::string, SyntaxKind> SyntaxKindMap = {

    {"\0", SyntaxKind::EndOfFileToken},
    {" ", SyntaxKind::WhiteSpaceToken},
    {"+", SyntaxKind::PlusToken},
    {"+=", SyntaxKind::PlusEqualsToken},
    {"-", SyntaxKind::MinusToken},
    {"-=", SyntaxKind::MinusEqualsToken},
    {"*", SyntaxKind::StarToken},
    {"*=", SyntaxKind::StarEqualsToken},
    {"/", SyntaxKind::SlashToken},
    {"/=", SyntaxKind::SlashEqualsToken},
    {"(", SyntaxKind::OpenParenthesisToken},
    {")", SyntaxKind::CloseParenthesisToken},
    {"[", SyntaxKind::OpenBracketToken},
    {"]", SyntaxKind::CloseBracketToken},
    {"{", SyntaxKind::OpenBraceToken},
    {"}", SyntaxKind::CloseBraceToken},
    {":", SyntaxKind::ColonToken},
    {",", SyntaxKind::CommaToken},
    {"~", SyntaxKind::TildeToken},
    {"^", SyntaxKind::HatToken},
    {"^=", SyntaxKind::HatEqualsToken},
    {"&&", SyntaxKind::AmpersandAmpersandToken},
    {"&=", SyntaxKind::AmpersandEqualsToken},
    {"&", SyntaxKind::AmpersandToken},
    {"||", SyntaxKind::PipePipeToken},
    {"|=", SyntaxKind::PipeEqualsToken},
    {"|", SyntaxKind::PipeToken},
    {"=", SyntaxKind::EqualsToken},
    {"==", SyntaxKind::EqualsEqualsToken},
    {"!", SyntaxKind::BangToken},
    {"!=", SyntaxKind::BangEqualsToken},
    {"<", SyntaxKind::LessToken},
    {"<=", SyntaxKind::LessOrEqualsToken},
    {">", SyntaxKind::GreaterToken},
    {">=", SyntaxKind::GreaterOrEqualsToken},
};

static const std::unordered_map<SyntaxKind, std::string, SyntaxKind::Hash>
    SyntaxTokenToStrMap = {
        {SyntaxKind::EndOfFileToken, "\0"},
        {SyntaxKind::WhiteSpaceToken, " "},
        {SyntaxKind::PlusToken, "+"},
        {SyntaxKind::PlusEqualsToken, "+="},
        {SyntaxKind::MinusToken, "-"},
        {SyntaxKind::MinusEqualsToken, "-="},
        {SyntaxKind::StarToken, "*"},
        {SyntaxKind::StarEqualsToken, "*="},
        {SyntaxKind::SlashToken, "/"},
        {SyntaxKind::SlashEqualsToken, "/="},
        {SyntaxKind::OpenParenthesisToken, "("},
        {SyntaxKind::CloseParenthesisToken, ")"},
        {SyntaxKind::OpenBracketToken, "["},
        {SyntaxKind::CloseBracketToken, "]"},
        {SyntaxKind::OpenBraceToken, "{"},
        {SyntaxKind::CloseBraceToken, "}"},
        {SyntaxKind::ColonToken, ":"},
        {SyntaxKind::CommaToken, ","},
        {SyntaxKind::TildeToken, "~"},
        {SyntaxKind::HatToken, "^"},
        {SyntaxKind::HatEqualsToken, "^="},
        {SyntaxKind::AmpersandAmpersandToken, "&&"},
        {SyntaxKind::AmpersandEqualsToken, "&="},
        {SyntaxKind::AmpersandToken, "&"},
        {SyntaxKind::PipePipeToken, "||"},
        {SyntaxKind::PipeEqualsToken, "|="},
        {SyntaxKind::PipeToken, "|"},
        {SyntaxKind::EqualsToken, "="},
        {SyntaxKind::EqualsEqualsToken, "=="},
        {SyntaxKind::BangToken, "!"},
        {SyntaxKind::BangEqualsToken, "!="},
        {SyntaxKind::LessToken, "<"},
        {SyntaxKind::LessOrEqualsToken, "<="},
        {SyntaxKind::GreaterToken, ">"},
        {SyntaxKind::GreaterOrEqualsToken, ">="},
};

static const std::unordered_map<SyntaxKind, std::string, SyntaxKind::Hash>
    SyntaxKindStrMap = {
        {SyntaxKind::EndOfFileToken, "EndOfFileToken"},
        {SyntaxKind::WhiteSpaceToken, "WhiteSpaceToken"},
        {SyntaxKind::PlusToken, "PlusToken"},
        {SyntaxKind::PlusEqualsToken, "PlusEqualsToken"},
        {SyntaxKind::MinusToken, "MinusToken"},
        {SyntaxKind::MinusEqualsToken, "MinusEqualsToken"},
        {SyntaxKind::StarToken, "StarToken"},
        {SyntaxKind::StarEqualsToken, "StarEqualsToken"},
        {SyntaxKind::SlashToken, "SlashToken"},
        {SyntaxKind::SlashEqualsToken, "SlashEqualsToken"},
        {SyntaxKind::OpenParenthesisToken, "OpenParenthesisToken"},
        {SyntaxKind::CloseParenthesisToken, "CloseParenthesisToken"},
        {SyntaxKind::OpenBracketToken, "OpenBracketToken"},
        {SyntaxKind::CloseBracketToken, "CloseBracketToken"},
        {SyntaxKind::OpenBraceToken, "OpenBraceToken"},
        {SyntaxKind::CloseBraceToken, "CloseBraceToken"},
        {SyntaxKind::ColonToken, "ColonToken"},
        {SyntaxKind::CommaToken, "CommaToken"},
        {SyntaxKind::TildeToken, "TildeToken"},
        {SyntaxKind::HatToken, "HatToken"},
        {SyntaxKind::HatEqualsToken, "HatEqualsToken"},
        {SyntaxKind::AmpersandAmpersandToken, "AmpersandAmpersandToken"},
        {SyntaxKind::AmpersandEqualsToken, "AmpersandEqualsToken"},
        {SyntaxKind::AmpersandToken, "AmpersandToken"},
        {SyntaxKind::PipePipeToken, "PipePipeToken"},
        {SyntaxKind::PipeEqualsToken, "PipeEqualsToken"},
        {SyntaxKind::PipeToken, "PipeToken"},
        {SyntaxKind::EqualsToken, "EqualsToken"},
        {SyntaxKind::EqualsEqualsToken, "EqualsEqualsToken"},
        {SyntaxKind::BangToken, "BangToken"},
        {SyntaxKind::BangEqualsToken, "BangEqualsToken"},
        {SyntaxKind::LessToken, "LessToken"},
        {SyntaxKind::LessOrEqualsToken, "LessOrEqualsToken"},
        {SyntaxKind::GreaterToken, "GreaterToken"},
        {SyntaxKind::GreaterOrEqualsToken, "GreaterOrEqualsToken"},
        {SyntaxKind::NumberToken, "NumberToken"},
        {SyntaxKind::BinaryExpression, "BinaryExpression"},
        {SyntaxKind::UnaryExpression, "UnaryExpression"},
        {SyntaxKind::ParenthesizedExpression, "ParenthesizedExpression"},
        {SyntaxKind::LiteralExpression, "LiteralExpression"},
        {SyntaxKind::AssignmentExpression, "AssignmentExpression"},
        {SyntaxKind::UnknownToken, "UnknownToken"},
        {SyntaxKind::IdentifierToken, "IdentifierToken"},
        {SyntaxKind::IdentifierExpression, "IdentifierExpression"},
        {SyntaxKind::BreakKeyword, "BreakKeyword"},
        {SyntaxKind::ContinueKeyword, "ContinueKeyword"},
        {SyntaxKind::IfKeyword, "IfKeyword"},
        {SyntaxKind::ElseKeyword, "ElseKeyword"},
        {SyntaxKind::TrueKeyword, "TrueKeyword"},
        {SyntaxKind::FalseKeyword, "FalseKeyword"},
        {SyntaxKind::ForKeyword, "ForKeyword"},
        {SyntaxKind::WhileKeyword, "WhileKeyword"},
        {SyntaxKind::DoKeyword, "DoKeyword"},
        {SyntaxKind::FunctionKeyword, "FunctionKeyword"},
        {SyntaxKind::ReturnKeyword, "ReturnKeyword"},
        {SyntaxKind::LetKeyword, "LetKeyword"},
        {SyntaxKind::VarKeyword, "VarKeyword"}};

inline std::ostream &operator<<(std::ostream &out, const SyntaxKind s) {
  out << SyntaxKindStrMap.at(s);
  return out;
}
