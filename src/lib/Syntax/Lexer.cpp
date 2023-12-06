// Copyright (c) 2022 F. Lotfi All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "Lexer.h"
#include "SyntaxKind.h"
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

void Lexer::ReadToken(SyntaxKind &type) {
  switch (CurrentToken()) {
  case '\0':
    type = SyntaxKind::EndOfFileToken;
    break;
  case ' ':
    type = SyntaxKind::WhiteSpaceToken;
    mPosition++;
    break;
  case '+':
    mPosition++;
    if (CurrentToken() != '=') {
      type = SyntaxKind::PlusToken;
    } else {
      type = SyntaxKind::PlusEqualsToken;
      mPosition++;
    }
    break;
  case '-':
    mPosition++;
    if (CurrentToken() != '=') {
      type = SyntaxKind::MinusToken;
    } else {
      type = SyntaxKind::MinusEqualsToken;
      mPosition++;
    }
    break;
  case '*':
    mPosition++;
    if (CurrentToken() != '=') {
      type = SyntaxKind::StarToken;
    } else {
      type = SyntaxKind::StarEqualsToken;
      mPosition++;
    }
    break;
  case '/':
    mPosition++;
    if (CurrentToken() != '=') {
      type = SyntaxKind::SlashToken;
    } else {
      type = SyntaxKind::SlashEqualsToken;
      mPosition++;
    }
    break;
  case '(':
    type = SyntaxKind::OpenParenthesisToken;
    mPosition++;
    break;
  case ')':
    type = SyntaxKind::CloseParenthesisToken;
    mPosition++;
    break;
  case '{':
    type = SyntaxKind::OpenBraceToken;
    mPosition++;
    break;
  case '}':
    type = SyntaxKind::CloseBraceToken;
    mPosition++;
    break;
  case ':':
    type = SyntaxKind::ColonToken;
    mPosition++;
    break;
  case ',':
    type = SyntaxKind::CommaToken;
    mPosition++;
    break;
  case '~':
    type = SyntaxKind::TildeToken;
    mPosition++;
    break;
  case '^':
    mPosition++;
    if (CurrentToken() != '=') {
      type = SyntaxKind::HatToken;
    } else {
      type = SyntaxKind::HatEqualsToken;
      mPosition++;
    }
    break;
  case '&':
    mPosition++;
    if (CurrentToken() == '&') {
      type = SyntaxKind::AmpersandAmpersandToken;
      mPosition++;
    } else if (CurrentToken() == '=') {
      type = SyntaxKind::AmpersandEqualsToken;
      mPosition++;
    } else {
      type = SyntaxKind::AmpersandToken;
    }
    break;
  case '|':
    mPosition++;
    if (CurrentToken() == '|') {
      type = SyntaxKind::PipePipeToken;
      mPosition++;
    } else if (CurrentToken() == '=') {
      type = SyntaxKind::PipeEqualsToken;
      mPosition++;
    } else {
      type = SyntaxKind::PipeToken;
    }
    break;
  case '=':
    mPosition++;
    if (CurrentToken() != '=') {
      type = SyntaxKind::EqualsToken;
    } else {
      type = SyntaxKind::EqualsEqualsToken;
      mPosition++;
    }
    break;
  case '!':
    mPosition++;
    if (CurrentToken() != '=') {
      type = SyntaxKind::BangToken;
    } else {
      type = SyntaxKind::BangEqualsToken;
      mPosition++;
    }
    break;
  case '<':
    mPosition++;
    if (CurrentToken() != '=') {
      type = SyntaxKind::LessToken;
    } else {
      type = SyntaxKind::LessOrEqualsToken;
      mPosition++;
    }
    break;
  case '>':
    mPosition++;
    if (CurrentToken() != '=') {
      type = SyntaxKind::GreaterToken;
    } else {
      type = SyntaxKind::GreaterOrEqualsToken;
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
void Lexer::ParseLetters(SyntaxKind &type) {
  if (isalpha(CurrentToken())) {
    ReadIdentifierOrKeyword(type);
  }
}

void Lexer::ReadIdentifierOrKeyword(SyntaxKind &type) {
  int32_t start = mPosition;
  while (isalpha(CurrentToken()) || isdigit(CurrentToken()) ||
         CurrentToken() == '_') {
    mPosition++;
  }
  int32_t length = mPosition - start;
  std::string text = mText.substr(start, length);
  type = SyntaxKind::GetKeywordKind(text);
  if (type == SyntaxKind::TrueKeyword) {
    mValue = true;
  }
  if (type == SyntaxKind::FalseKeyword) {
    mValue = false;
  }
  if (type == SyntaxKind::IdentifierToken) {
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
void Lexer::ParseNumber(SyntaxKind &type) {
  int32_t posStart = mPosition;
  while (isdigit(CurrentToken())) {
    mPosition++;
  }
  const char *startPointer = mText.c_str() + posStart;
  std::string numberStr(startPointer, mPosition - posStart);
  int64_t val = static_cast<int64_t>(std::stoll(numberStr, nullptr, 10));

  if (val <= static_cast<int64_t>(std::numeric_limits<int32_t>::min())) {
    // underflow
    mRecords.ReportUnderflow(posStart, mPosition, val);
    mValue = 0;
    type = SyntaxKind::NumberToken;
    return;
  } else if (val >= static_cast<int64_t>(std::numeric_limits<int32_t>::max())) {
    // overflow
    mRecords.ReportOverflow(posStart, mPosition, val);
    mValue = 0;
    type = SyntaxKind::NumberToken;
    return;
  }
  mValue = static_cast<int32_t>(val);
  type = SyntaxKind::NumberToken;
}

std::unique_ptr<SyntaxToken> Lexer::NextToken() {
  SyntaxKind type = SyntaxKind::UnknownToken;
  int32_t tokenStartPos = mPosition;
  ReadToken(type);
  int32_t tokenEndPos = mPosition;
  if (type == SyntaxKind::NumberToken || type == SyntaxKind::TrueKeyword ||
      type == SyntaxKind::FalseKeyword) {
    return std::make_unique<SyntaxToken>(type, tokenStartPos, tokenEndPos,
                                         mValue);
  }
  if (type == SyntaxKind::IdentifierToken) {
    return std::make_unique<SyntaxToken>(type, tokenStartPos, tokenEndPos,
                                         mIdentifier);
  }
  if (type == SyntaxKind::UnknownToken) {
    mRecords.ReportBadCharacter(mPosition, CurrentToken());
    mPosition++;
    return std::make_unique<SyntaxToken>(type, tokenStartPos, tokenEndPos,
                                         std::string() + CurrentToken());
  }
  return std::make_unique<SyntaxToken>(type, tokenStartPos, tokenEndPos,
                                       SyntaxTokenToStrMap.at(type));
}