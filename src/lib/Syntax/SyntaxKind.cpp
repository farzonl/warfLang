// Copyright (c) 2022 F. Lotfi All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "SyntaxKind.h"

SyntaxKind SyntaxKind::GetKeywordKind(std::string &text) {
  auto it = KeywordMap.find(text);
  if (it != KeywordMap.end()) {
    return it->second;
  }
  return SyntaxKind::IdentifierToken;
}

int SyntaxOrder::GetUnaryOperatorPrecedence(SyntaxKind type) {
  switch (type.GetValue()) {
  case SyntaxKind::PlusToken:
  case SyntaxKind::MinusToken:
  case SyntaxKind::BangToken:
  case SyntaxKind::TildeToken:
    return 6;
  default:
    return 0;
  }
}

int SyntaxOrder::GetBinaryOperatorPrecedence(SyntaxKind type) {
  switch (type.GetValue()) {
  case SyntaxKind::StarToken:
  case SyntaxKind::SlashToken:
    return 5;
  case SyntaxKind::PlusToken:
  case SyntaxKind::MinusToken:
    return 4;
  case SyntaxKind::EqualsEqualsToken:
  case SyntaxKind::BangEqualsToken:
  case SyntaxKind::LessToken:
  case SyntaxKind::LessOrEqualsToken:
  case SyntaxKind::GreaterToken:
  case SyntaxKind::GreaterOrEqualsToken:
    return 3;
  case SyntaxKind::AmpersandToken:
  case SyntaxKind::AmpersandAmpersandToken:
    return 2;
  case SyntaxKind::PipeToken:
  case SyntaxKind::PipePipeToken:
  case SyntaxKind::HatToken:
    return 1;
  default:
    return 0;
  }
}