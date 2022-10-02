/*
 * Copyright (c) 2022 F. Lotfi All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#pragma once
#include <cassert>
#include <iostream>
#include <string>
#include <unordered_map>

class SyntaxType {
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
    // Nodes
    CompilationUnit,
    // Expressions
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

  SyntaxType() : value(UnknownToken) {}
  constexpr SyntaxType(Value aSyntaxType) : value(aSyntaxType) {}

  // Allow switch and comparisons.
  Value GetValue() const { return value; }
  // Note: I want to do this but it conflicts with operator ==
  // constexpr operator Value() const { return value; }

  // Prevent usage: if(SyntaxType)
  explicit operator bool() const = delete;
  constexpr bool operator==(SyntaxType a) const { return value == a.value; }
  constexpr bool operator!=(SyntaxType a) const { return value != a.value; }

  static SyntaxType GetKeywordType(std::string &text);

  struct Hash {
    size_t operator()(const SyntaxType &a) const {
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
  static int GetUnaryOperatorPrecedence(SyntaxType type);
  static int GetBinaryOperatorPrecedence(SyntaxType type);

private:
  SyntaxOrder() = delete;
};

static const std::unordered_map<std::string, SyntaxType> KeywordMap = {
    {"break", SyntaxType::BreakKeyword},
    {"continue", SyntaxType::ContinueKeyword},
    {"if", SyntaxType::IfKeyword},
    {"else", SyntaxType::ElseKeyword},
    {"true", SyntaxType::TrueKeyword},
    {"false", SyntaxType::FalseKeyword},
    {"for", SyntaxType::ForKeyword},
    {"while", SyntaxType::WhileKeyword},
    {"do", SyntaxType::DoKeyword},
    {"function", SyntaxType::FunctionKeyword},
    {"return", SyntaxType::ReturnKeyword},
    {"let", SyntaxType::LetKeyword},
    {"var", SyntaxType::VarKeyword},
};

static const std::unordered_map<SyntaxType, std::string, SyntaxType::Hash>
    KeywordToStrMap = {
        {SyntaxType::BreakKeyword, "break"},
        {SyntaxType::ContinueKeyword, "continue"},
        {SyntaxType::IfKeyword, "if"},
        {SyntaxType::ElseKeyword, "else"},
        {SyntaxType::TrueKeyword, "true"},
        {SyntaxType::FalseKeyword, "false"},
        {SyntaxType::ForKeyword, "for"},
        {SyntaxType::WhileKeyword, "while"},
        {SyntaxType::DoKeyword, "do"},
        {SyntaxType::FunctionKeyword, "function"},
        {SyntaxType::ReturnKeyword, "return"},
        {SyntaxType::LetKeyword, "let"},
        {SyntaxType::VarKeyword, "var"},
};

static const std::unordered_map<std::string, SyntaxType> SyntaxTypeMap = {

    {"\0", SyntaxType::EndOfFileToken},
    {" ", SyntaxType::WhiteSpaceToken},
    {"+", SyntaxType::PlusToken},
    {"+=", SyntaxType::PlusEqualsToken},
    {"-", SyntaxType::MinusToken},
    {"-=", SyntaxType::MinusEqualsToken},
    {"*", SyntaxType::StarToken},
    {"*=", SyntaxType::StarEqualsToken},
    {"/", SyntaxType::SlashToken},
    {"/=", SyntaxType::SlashEqualsToken},
    {"(", SyntaxType::OpenParenthesisToken},
    {")", SyntaxType::CloseParenthesisToken},
    {"[", SyntaxType::OpenBracketToken},
    {"]", SyntaxType::CloseBracketToken},
    {"{", SyntaxType::OpenBraceToken},
    {"}", SyntaxType::CloseBraceToken},
    {":", SyntaxType::ColonToken},
    {",", SyntaxType::CommaToken},
    {"~", SyntaxType::TildeToken},
    {"^", SyntaxType::HatToken},
    {"^=", SyntaxType::HatEqualsToken},
    {"&&", SyntaxType::AmpersandAmpersandToken},
    {"&=", SyntaxType::AmpersandEqualsToken},
    {"&", SyntaxType::AmpersandToken},
    {"||", SyntaxType::PipePipeToken},
    {"|=", SyntaxType::PipeEqualsToken},
    {"|", SyntaxType::PipeToken},
    {"=", SyntaxType::EqualsToken},
    {"==", SyntaxType::EqualsEqualsToken},
    {"!", SyntaxType::BangToken},
    {"!=", SyntaxType::BangEqualsToken},
    {"<", SyntaxType::LessToken},
    {"<=", SyntaxType::LessOrEqualsToken},
    {">", SyntaxType::GreaterToken},
    {">=", SyntaxType::GreaterOrEqualsToken},
};

static const std::unordered_map<SyntaxType, std::string, SyntaxType::Hash>
    SyntaxTokenToStrMap = {
        {SyntaxType::EndOfFileToken, "\0"},
        {SyntaxType::WhiteSpaceToken, " "},
        {SyntaxType::PlusToken, "+"},
        {SyntaxType::PlusEqualsToken, "+="},
        {SyntaxType::MinusToken, "-"},
        {SyntaxType::MinusEqualsToken, "-="},
        {SyntaxType::StarToken, "*"},
        {SyntaxType::StarEqualsToken, "*="},
        {SyntaxType::SlashToken, "/"},
        {SyntaxType::SlashEqualsToken, "/="},
        {SyntaxType::OpenParenthesisToken, "("},
        {SyntaxType::CloseParenthesisToken, ")"},
        {SyntaxType::OpenBracketToken, "["},
        {SyntaxType::CloseBracketToken, "]"},
        {SyntaxType::OpenBraceToken, "{"},
        {SyntaxType::CloseBraceToken, "}"},
        {SyntaxType::ColonToken, ":"},
        {SyntaxType::CommaToken, ","},
        {SyntaxType::TildeToken, "~"},
        {SyntaxType::HatToken, "^"},
        {SyntaxType::HatEqualsToken, "^="},
        {SyntaxType::AmpersandAmpersandToken, "&&"},
        {SyntaxType::AmpersandEqualsToken, "&="},
        {SyntaxType::AmpersandToken, "&"},
        {SyntaxType::PipePipeToken, "||"},
        {SyntaxType::PipeEqualsToken, "|="},
        {SyntaxType::PipeToken, "|"},
        {SyntaxType::EqualsToken, "="},
        {SyntaxType::EqualsEqualsToken, "=="},
        {SyntaxType::BangToken, "!"},
        {SyntaxType::BangEqualsToken, "!="},
        {SyntaxType::LessToken, "<"},
        {SyntaxType::LessOrEqualsToken, "<="},
        {SyntaxType::GreaterToken, ">"},
        {SyntaxType::GreaterOrEqualsToken, ">="},
};

static const std::unordered_map<SyntaxType, std::string, SyntaxType::Hash>
    SyntaxTypeStrMap = {
        {SyntaxType::EndOfFileToken, "EndOfFileToken"},
        {SyntaxType::WhiteSpaceToken, "WhiteSpaceToken"},
        {SyntaxType::PlusToken, "PlusToken"},
        {SyntaxType::PlusEqualsToken, "PlusEqualsToken"},
        {SyntaxType::MinusToken, "MinusToken"},
        {SyntaxType::MinusEqualsToken, "MinusEqualsToken"},
        {SyntaxType::StarToken, "StarToken"},
        {SyntaxType::StarEqualsToken, "StarEqualsToken"},
        {SyntaxType::SlashToken, "SlashToken"},
        {SyntaxType::SlashEqualsToken, "SlashEqualsToken"},
        {SyntaxType::OpenParenthesisToken, "OpenParenthesisToken"},
        {SyntaxType::CloseParenthesisToken, "CloseParenthesisToken"},
        {SyntaxType::OpenBracketToken, "OpenBracketToken"},
        {SyntaxType::CloseBracketToken, "CloseBracketToken"},
        {SyntaxType::OpenBraceToken, "OpenBraceToken"},
        {SyntaxType::CloseBraceToken, "CloseBraceToken"},
        {SyntaxType::ColonToken, "ColonToken"},
        {SyntaxType::CommaToken, "CommaToken"},
        {SyntaxType::TildeToken, "TildeToken"},
        {SyntaxType::HatToken, "HatToken"},
        {SyntaxType::HatEqualsToken, "HatEqualsToken"},
        {SyntaxType::AmpersandAmpersandToken, "AmpersandAmpersandToken"},
        {SyntaxType::AmpersandEqualsToken, "AmpersandEqualsToken"},
        {SyntaxType::AmpersandToken, "AmpersandToken"},
        {SyntaxType::PipePipeToken, "PipePipeToken"},
        {SyntaxType::PipeEqualsToken, "PipeEqualsToken"},
        {SyntaxType::PipeToken, "PipeToken"},
        {SyntaxType::EqualsToken, "EqualsToken"},
        {SyntaxType::EqualsEqualsToken, "EqualsEqualsToken"},
        {SyntaxType::BangToken, "BangToken"},
        {SyntaxType::BangEqualsToken, "BangEqualsToken"},
        {SyntaxType::LessToken, "LessToken"},
        {SyntaxType::LessOrEqualsToken, "LessOrEqualsToken"},
        {SyntaxType::GreaterToken, "GreaterToken"},
        {SyntaxType::GreaterOrEqualsToken, "GreaterOrEqualsToken"},
        {SyntaxType::NumberToken, "NumberToken"},
        {SyntaxType::BinaryExpression, "BinaryExpression"},
        {SyntaxType::UnaryExpression, "UnaryExpression"},
        {SyntaxType::ParenthesizedExpression, "ParenthesizedExpression"},
        {SyntaxType::LiteralExpression, "LiteralExpression"},
        {SyntaxType::AssignmentExpression, "AssignmentExpression"},
        {SyntaxType::UnknownToken, "UnknownToken"},
        {SyntaxType::CompilationUnit, "CompilationUnit"},
        {SyntaxType::IdentifierToken, "IdentifierToken"},
        {SyntaxType::IdentifierExpression, "IdentifierExpression"},
        {SyntaxType::BreakKeyword, "BreakKeyword"},
        {SyntaxType::ContinueKeyword, "ContinueKeyword"},
        {SyntaxType::IfKeyword, "IfKeyword"},
        {SyntaxType::ElseKeyword, "ElseKeyword"},
        {SyntaxType::TrueKeyword, "TrueKeyword"},
        {SyntaxType::FalseKeyword, "FalseKeyword"},
        {SyntaxType::ForKeyword, "ForKeyword"},
        {SyntaxType::WhileKeyword, "WhileKeyword"},
        {SyntaxType::DoKeyword, "DoKeyword"},
        {SyntaxType::FunctionKeyword, "FunctionKeyword"},
        {SyntaxType::ReturnKeyword, "ReturnKeyword"},
        {SyntaxType::LetKeyword, "LetKeyword"},
        {SyntaxType::VarKeyword, "VarKeyword"}};

inline std::ostream &operator<<(std::ostream &out, const SyntaxType s) {
  out << SyntaxTypeStrMap.at(s);
  return out;
}
