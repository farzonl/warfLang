#include "SyntaxToken.h"

const Value &SyntaxToken::GetValue() const { return mValue; }

bool SyntaxToken::HasValue() const { return mValue.type != Type::Unknown; }

int SyntaxToken::Position() const { return mPosition; }

std::string SyntaxToken::Text() const { return mText; }

SyntaxToken::SyntaxToken(SyntaxType synType, int pos, std::string text)
    : SyntaxNode(synType), mPosition(pos), mText(text) {}

SyntaxToken::SyntaxToken(SyntaxType synType, int pos, Value value)
    : SyntaxNode(synType), mPosition(pos), mText(), mValue(value) {}