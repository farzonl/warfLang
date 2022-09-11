// Copyright (c) 2022 F. Lotfi All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "Lexer.h"
#include "SyntaxType.h"
#include <errno.h>
#include <limits>
#include <sstream>
#include <stdlib.h>
#include <string.h>

Lexer::Lexer(std::string text) : mText(text), mPosition(0), mRecords("Lexer") {}

char Lexer::CurrentToken() {
  if (mPosition >= mText.length()) {
    return '\0';
  }
  return mText[mPosition];
}

void Lexer::ReadToken(SyntaxType &type) {
  switch (CurrentToken()) {
  case '\0':
    type = SyntaxType::EndOfFileToken;
    break;
  case ' ':
    type = SyntaxType::WhiteSpaceToken;
    mPosition++;
    break;
  case '+':
    mPosition++;
    if (CurrentToken() != '=') {
      type = SyntaxType::PlusToken;
    } else {
      type = SyntaxType::PlusEqualsToken;
      mPosition++;
    }
    break;
  case '-':
    mPosition++;
    if (CurrentToken() != '=') {
      type = SyntaxType::MinusToken;
    } else {
      type = SyntaxType::MinusEqualsToken;
      mPosition++;
    }
    break;
  case '*':
    mPosition++;
    if (CurrentToken() != '=') {
      type = SyntaxType::StarToken;
    } else {
      type = SyntaxType::StarEqualsToken;
      mPosition++;
    }
    break;
  case '/':
    mPosition++;
    if (CurrentToken() != '=') {
      type = SyntaxType::SlashToken;
    } else {
      type = SyntaxType::SlashEqualsToken;
      mPosition++;
    }
    break;
  case '(':
    type = SyntaxType::OpenParenthesisToken;
    mPosition++;
    break;
  case ')':
    type = SyntaxType::CloseParenthesisToken;
    mPosition++;
    break;
  case '{':
    type = SyntaxType::OpenBraceToken;
    mPosition++;
    break;
  case '}':
    type = SyntaxType::CloseBraceToken;
    mPosition++;
    break;
  case ':':
    type = SyntaxType::ColonToken;
    mPosition++;
    break;
  case ',':
    type = SyntaxType::CommaToken;
    mPosition++;
    break;
  case '~':
    type = SyntaxType::TildeToken;
    mPosition++;
    break;
  case '^':
    mPosition++;
    if (CurrentToken() != '=') {
      type = SyntaxType::HatToken;
    } else {
      type = SyntaxType::HatEqualsToken;
      mPosition++;
    }
    break;
  case '&':
    mPosition++;
    if (CurrentToken() == '&') {
      type = SyntaxType::AmpersandAmpersandToken;
      mPosition++;
    } else if (CurrentToken() == '=') {
      type = SyntaxType::AmpersandEqualsToken;
      mPosition++;
    } else {
      type = SyntaxType::AmpersandToken;
    }
    break;
  case '|':
    mPosition++;
    if (CurrentToken() == '|') {
      type = SyntaxType::PipePipeToken;
      mPosition++;
    } else if (CurrentToken() == '=') {
      type = SyntaxType::PipeEqualsToken;
      mPosition++;
    } else {
      type = SyntaxType::PipeToken;
    }
    break;
  case '=':
    mPosition++;
    if (CurrentToken() != '=') {
      type = SyntaxType::EqualsToken;
    } else {
      type = SyntaxType::EqualsEqualsToken;
      mPosition++;
    }
    break;
  case '!':
    mPosition++;
    if (CurrentToken() != '=') {
      type = SyntaxType::BangToken;
    } else {
      type = SyntaxType::BangEqualsToken;
      mPosition++;
    }
    break;
  case '<':
    mPosition++;
    if (CurrentToken() != '=') {
      type = SyntaxType::LessToken;
    } else {
      type = SyntaxType::LessOrEqualsToken;
      mPosition++;
    }
    break;
  case '>':
    mPosition++;
    if (CurrentToken() != '=') {
      type = SyntaxType::GreaterToken;
    } else {
      type = SyntaxType::GreaterOrEqualsToken;
      mPosition++;
    }
    break;
  case '0':
  case '1':
  case '2':
  case '3':
  case '4':
  case '5':
  case '6':
  case '7':
  case '8':
  case '9':
    ParseNumber(type);
    break;
  case '_':
    ReadIdentifierOrKeyword(type);
    break;
  default:
    ParseLetters(type);
  }
}
void Lexer::ParseLetters(SyntaxType &type) {
  if (isalpha(CurrentToken())) {
    ReadIdentifierOrKeyword(type);
  }
}

void Lexer::ReadIdentifierOrKeyword(SyntaxType &type) {
  int32_t start = mPosition;
  while (isalpha(CurrentToken()) || isdigit(CurrentToken()) ||
         CurrentToken() == '_') {
    mPosition++;
  }
  int32_t length = mPosition - start;
  std::string text = mText.substr(start, length);
  type = SyntaxType::GetKeywordType(text);
  if (type == SyntaxType::TrueKeyword) {
    mValue = true;
  }
  if (type == SyntaxType::FalseKeyword) {
    mValue = false;
  }
  if (type == SyntaxType::IdentifierToken) {
    mIdentifier = text;
  }
}
// Note because we are just parsing numeric numbers
// we don't know if a sign had been added to a literal.
// that means we are not properly setting int32_t min\max
// INT_MIN : -2147483648 INT_MAX : 2147483647.
// for example -2147483648 would be interpreted as 2147483648
// which we would treat as an overflow.
// This has two issues:
//   1) we are reporting underflows as overflows
//   2) we have an off by one error, where a valid int is now not possible.
void Lexer::ParseNumber(SyntaxType &type) {
  int32_t posStart = mPosition;
  while (isdigit(CurrentToken())) {
    mPosition++;
  }
  const char *startPointer = mText.c_str() + posStart;
  const char *endPointer = mText.c_str() + mPosition;
  int64_t val = static_cast<int64_t>(
      strtol(startPointer, const_cast<char **>(&endPointer), 10));
  ;

  if (val <= static_cast<int64_t>(std::numeric_limits<int32_t>::min())) {
    // underflow
    mRecords.ReportUnderflow(posStart, mPosition, val);
    mValue = 0;
    type = SyntaxType::NumberToken;
    return;
  } else if (val >= static_cast<int64_t>(std::numeric_limits<int32_t>::max())) {
    // overflow
    mRecords.ReportOverflow(posStart, mPosition, val);
    mValue = 0;
    type = SyntaxType::NumberToken;
    return;
  }
  mValue = static_cast<int32_t>(val);
  type = SyntaxType::NumberToken;
}

std::unique_ptr<SyntaxToken> Lexer::NextToken() {
  SyntaxType type = SyntaxType::UnknownToken;
  int32_t tokenStartPos = mPosition;
  ReadToken(type);
  int32_t tokenEndPos = mPosition;
  if (type == SyntaxType::NumberToken || type == SyntaxType::TrueKeyword ||
      type == SyntaxType::FalseKeyword) {
    return std::make_unique<SyntaxToken>(type, tokenStartPos, tokenEndPos,
                                         mValue);
  }
  if (type == SyntaxType::IdentifierToken) {
    return std::make_unique<SyntaxToken>(type, tokenStartPos, tokenEndPos,
                                         mIdentifier);
  }
  if (type == SyntaxType::UnknownToken) {
    mRecords.ReportBadCharacter(mPosition, CurrentToken());
    mPosition++;
    return std::make_unique<SyntaxToken>(type, tokenStartPos, tokenEndPos,
                                         std::string() + CurrentToken());
  }
  return std::make_unique<SyntaxToken>(type, tokenStartPos, tokenEndPos,
                                       SyntaxTokenToStrMap.at(type));
}