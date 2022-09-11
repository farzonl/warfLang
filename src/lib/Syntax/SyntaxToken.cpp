// Copyright (c) 2022 F. Lotfi All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "SyntaxToken.h"

const Value &SyntaxToken::GetValue() const { return mValue; }

bool SyntaxToken::HasValue() const { return mValue.type != Type::Unknown; }

const TextSpan &SyntaxToken::Span() const { return mTextSpan; }

std::string SyntaxToken::Text() const { return mText; }

SyntaxToken::SyntaxToken(SyntaxType synType, int startPos, int endPos,
                         std::string text)
    : SyntaxNode(synType), mTextSpan(startPos, endPos), mText(text) {}

SyntaxToken::SyntaxToken(SyntaxType synType, int startPos, int endPos,
                         Value value)
    : SyntaxNode(synType), mTextSpan(startPos, endPos), mText(), mValue(value) {
}