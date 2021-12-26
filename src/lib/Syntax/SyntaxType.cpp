#include "SyntaxType.h"

int SyntaxOrder::GetUnaryOperatorPrecedence(SyntaxType type) {
  switch (type) {
  case SyntaxType::PlusToken:
  case SyntaxType::MinusToken:
  case SyntaxType::BangToken:
  case SyntaxType::TildeToken:
    return 6;
  default:
    return 0;
  }
}

int SyntaxOrder::GetBinaryOperatorPrecedence(SyntaxType type) {
  switch (type) {
  case SyntaxType::StarToken:
  case SyntaxType::SlashToken:
    return 5;
  case SyntaxType::PlusToken:
  case SyntaxType::MinusToken:
    return 4;
  case SyntaxType::EqualsEqualsToken:
  case SyntaxType::BangEqualsToken:
  case SyntaxType::LessToken:
  case SyntaxType::LessOrEqualsToken:
  case SyntaxType::GreaterToken:
  case SyntaxType::GreaterOrEqualsToken:
    return 3;
  case SyntaxType::AmpersandToken:
  case SyntaxType::AmpersandAmpersandToken:
    return 2;
  case SyntaxType::PipeToken:
  case SyntaxType::PipePipeToken:
  case SyntaxType::HatToken:
    return 1;
  default:
    return 0;
  }
}