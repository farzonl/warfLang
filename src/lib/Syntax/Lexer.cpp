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
  case '\t':
  case '\r':
  case '\n':
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
    if (CurrentToken() == '/') {
      // Line comment: consume the rest of the line.
      while (CurrentToken() != '\0' && CurrentToken() != '\n') {
        mPosition++;
      }
      type = SyntaxKind::CommentToken;
    } else if (CurrentToken() != '=') {
      type = SyntaxKind::SlashToken;
    } else {
      type = SyntaxKind::SlashEqualsToken;
      mPosition++;
    }
    break;
  case '%':
    mPosition++;
    if (CurrentToken() != '=') {
      type = SyntaxKind::PercentToken;
    } else {
      type = SyntaxKind::PercentEqualsToken;
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
  case '[':
    type = SyntaxKind::OpenBracketToken;
    mPosition++;
    break;
  case ']':
    type = SyntaxKind::CloseBracketToken;
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
  case ';':
    type = SyntaxKind::SemicolonToken;
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
  case '"':
    ParseString(type);
    break;
  default:
    ParseLetters(type);
  }
}
void Lexer::ParseLetters(SyntaxKind &type) {
  if (isalpha(static_cast<unsigned char>(CurrentToken()))) {
    ReadIdentifierOrKeyword(type);
  }
}

void Lexer::ReadIdentifierOrKeyword(SyntaxKind &type) {
  int32_t start = mPosition;
  while (isalpha(static_cast<unsigned char>(CurrentToken())) ||
         isdigit(static_cast<unsigned char>(CurrentToken())) ||
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
  while (isdigit(static_cast<unsigned char>(CurrentToken()))) {
    mPosition++;
  }
  const char *startPointer = mText.c_str() + posStart;
  if (CurrentToken() == '.' &&
      mPosition + 1 < static_cast<int32_t>(mText.length()) &&
      isdigit(static_cast<unsigned char>(mText[mPosition + 1]))) {
    mPosition++;
    while (isdigit(static_cast<unsigned char>(CurrentToken()))) {
      mPosition++;
    }
    std::string floatStr(startPointer, mPosition - posStart);
    double dval = 0.0;
    try {
      dval = std::stod(floatStr);
    } catch (const std::exception &) {
      dval = 0.0;
    }
    mValue = dval;
    type = SyntaxKind::FloatToken;
    return;
  }
  std::string numberStr(startPointer, mPosition - posStart);
  int64_t val = 0;
  try {
    val = static_cast<int64_t>(std::stoll(numberStr, nullptr, 10));
  } catch (const std::exception &) {
    mRecords.ReportOverflow(posStart, mPosition,
                            std::numeric_limits<int64_t>::max());
    mValue = 0;
    type = SyntaxKind::NumberToken;
    return;
  }

  if (val <= static_cast<int64_t>(std::numeric_limits<int32_t>::min())) {
    // underflow
    mRecords.ReportUnderflow(posStart, mPosition, val);
    mValue = 0;
    type = SyntaxKind::NumberToken;
    return;
  } else if (val >
             static_cast<int64_t>(std::numeric_limits<int32_t>::max()) + 1) {
    // strict overflow (> 2147483648)
    mRecords.ReportOverflow(posStart, mPosition, val);
    mValue = 0;
    type = SyntaxKind::NumberToken;
    return;
  } else if (val ==
             static_cast<int64_t>(std::numeric_limits<int32_t>::max()) + 1) {
    // val == 2147483648: tentative overflow (canceled if preceded by unary
    // minus)
    mRecords.ReportOverflow(posStart, mPosition, val);
    mValue = static_cast<int32_t>(val);
    type = SyntaxKind::NumberToken;
    return;
  }
  mValue = static_cast<int32_t>(val);
  type = SyntaxKind::NumberToken;
}

void Lexer::ParseString(SyntaxKind &type) {
  int32_t posStart = mPosition;
  mPosition++;
  std::string stringVal;
  bool closed = false;
  while (mPosition < static_cast<int32_t>(mText.length())) {
    char c = mText[mPosition];
    if (c == '"') {
      mPosition++;
      closed = true;
      break;
    }
    if (c == '\0' || c == '\n' || c == '\r') {
      break;
    }
    if (c == '\\') {
      mPosition++;
      if (mPosition >= static_cast<int32_t>(mText.length())) {
        break;
      }
      char esc = mText[mPosition];
      mPosition++;
      switch (esc) {
      case 'n':
        stringVal += '\n';
        break;
      case 't':
        stringVal += '\t';
        break;
      case '"':
        stringVal += '"';
        break;
      case '\\':
        stringVal += '\\';
        break;
      default:
        stringVal += '\\';
        stringVal += esc;
        break;
      }
    } else {
      stringVal += c;
      mPosition++;
    }
  }

  if (!closed) {
    mRecords.ReportUnterminatedString(TextSpan(posStart, mPosition));
  }
  mValue = stringVal;
  type = SyntaxKind::StringToken;
}

std::unique_ptr<SyntaxToken> Lexer::NextToken() {
  SyntaxKind type = SyntaxKind::UnknownToken;
  int32_t tokenStartPos = mPosition;
  ReadToken(type);
  int32_t tokenEndPos = mPosition;
  if (type == SyntaxKind::NumberToken || type == SyntaxKind::FloatToken ||
      type == SyntaxKind::TrueKeyword || type == SyntaxKind::FalseKeyword ||
      type == SyntaxKind::StringToken) {
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
