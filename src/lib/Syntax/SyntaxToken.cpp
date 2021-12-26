#include "SyntaxToken.h"

const ValueType &SyntaxToken::Value() const { return mValue; }

bool SyntaxToken::HasValue() const { return mValue.type != Type::UnknownType; }

int SyntaxToken::Position() const { return mPosition; }

std::string SyntaxToken::Text() const { return mText; }

SyntaxToken::SyntaxToken(SyntaxType synType, int pos, std::string text)
    : SyntaxNode(synType), mPosition(pos), mText(text) {}

SyntaxToken::SyntaxToken(SyntaxType synType, int pos, ValueType value)
    : SyntaxNode(synType), mPosition(pos), mText(), mValue(value) {}