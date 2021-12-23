#pragma once
#include <cassert>
#include <iostream>
#include <string>
#include <unordered_map>

enum class SyntaxType {
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
  UnaryExpression,
  BinaryExpression,
  ParenthesizedExpression,
  NumberNode
};

struct SyntaxOrder {
  static int GetUnaryOperatorPrecedence(SyntaxType type);
  static int GetBinaryOperatorPrecedence(SyntaxType type);

private:
  SyntaxOrder() = delete;
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

static const std::unordered_map<SyntaxType, std::string> SyntaxTokenToStrMap = {
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

static const std::unordered_map<SyntaxType, std::string> SyntaxTypeStrMap = {
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
    {SyntaxType::ParenthesizedExpression, "ParenthesizedExpression"},
    {SyntaxType::NumberNode, "NumberNode"},
    {SyntaxType::UnknownToken, "UnknownToken"}};

inline std::ostream &operator<<(std::ostream &out, const SyntaxType s) {
  out << SyntaxTypeStrMap.at(s);
  return out;
}
