#include "Lexer.h"
#include "SyntaxType.h"
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <sstream>
#include <stdlib.h>
#include <string.h>

Lexer::Lexer(std::string text) : mText(text), mPosition(0), mVecErrors() {}

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
  case 't':
  case 'f':
    ParseBool(type);
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
  }
}
void Lexer::ParseBool(SyntaxType &type) {

  const char *startPointer = mText.c_str() + mPosition;
  bool bValue;
  std::string trueStr = boolToNameMap.at(true);
  std::string falseStr = boolToNameMap.at(false);
  if (*startPointer == 't' && strncmp(startPointer, trueStr.c_str(),trueStr.size()) == 0) {
    bValue = true;
  } else if (*startPointer == 'f' && strncmp(startPointer, falseStr.c_str(), falseStr.size()) == 0) {
    bValue = false;
  } else {
    std::stringstream errorStream;
    errorStream << "LexerError: bad character input: " << CurrentToken();
    mVecErrors.push_back(errorStream.str());
    return;
  }
  std::string boolStr = boolToNameMap.at(bValue);
  std::string boolValueAsStr = mText.substr(mPosition, boolStr.size());
  // note only set the value if found.
  if (boolStrToValueMap.find(boolStr) != boolStrToValueMap.end()) {
    mValue = static_cast<bool>(bValue);
    type = SyntaxType::BooleanToken;
    mPosition += boolStr.size();
  }
}

void Lexer::ParseNumber(SyntaxType &type) {
  int32_t posStart = mPosition;
  while (isdigit(CurrentToken())) {
    mPosition++;
  }
  const char *startPointer = mText.c_str() + posStart;
  const char *endPointer = mText.c_str() + mPosition;
  long val = strtol(startPointer, const_cast<char **>(&endPointer), 10);
  if (val == LONG_MIN && errno == ERANGE) {
    // underflow
    mVecErrors.push_back("LexerError: Numeric underflow.");
    return;
  } else if (val == LONG_MAX && errno == ERANGE) {
    // overflow
    mVecErrors.push_back("LexerError: Numeric overflow.");
    return;
  }
  mValue = static_cast<int32_t>(val);
  type = SyntaxType::NumberToken;
}

std::unique_ptr<SyntaxToken> Lexer::NextToken() {
  SyntaxType type = SyntaxType::UnknownToken;
  int32_t tokenStartPos = mPosition;
  ReadToken(type);
  if (type == SyntaxType::NumberToken || type == SyntaxType::BooleanToken) {
    return std::make_unique<SyntaxToken>(type, tokenStartPos, mValue);
  }
  if (type == SyntaxType::UnknownToken) {
    std::stringstream errorStream;
    errorStream << "LexerError: bad character input: " << CurrentToken();
    mVecErrors.push_back(errorStream.str());
    mPosition++;
    return std::make_unique<SyntaxToken>(type, tokenStartPos,
                                         std::string() + CurrentToken());
  }
  return std::make_unique<SyntaxToken>(type, tokenStartPos,
                                       SyntaxTokenToStrMap.at(type));
}