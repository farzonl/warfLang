/*
 * Copyright (c) 2022 F. Lotfi All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#pragma once
#include <cassert>
#include <iostream>
#include <string>
#include <unordered_map>

#include "Binding/ValueType.h"
#include "Symbol/TextSpan.h"
#include "SyntaxNode.h"
#include "SyntaxType.h"

enum class Errors { LexError, ParseError };

class SyntaxToken : public SyntaxNode {
private:
  std::string mText;
  Value mValue;
  TextSpan mTextSpan;

public:
  bool HasValue() const;
  const Value &GetValue() const;
  const TextSpan &Span() const;
  std::string Text() const;
  SyntaxToken(SyntaxType synType, int startPos, int endPos, std::string text);
  SyntaxToken(SyntaxType synType, int startPos, int endPos, Value value);
  virtual ~SyntaxToken() {}
};