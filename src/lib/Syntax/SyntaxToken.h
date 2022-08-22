#pragma once
#include <cassert>
#include <iostream>
#include <string>
#include <unordered_map>

#include "Binding/ValueType.h"
#include "SyntaxNode.h"
#include "SyntaxType.h"

enum class Errors { LexError, ParseError };

class SyntaxToken : public SyntaxNode {
private:
  int mPosition;
  std::string mText;
  Value mValue;

public:
  bool HasValue() const;
  const Value &GetValue() const;
  int Position() const;
  std::string Text() const;
  SyntaxToken(SyntaxType synType, int pos, std::string text);
  SyntaxToken(SyntaxType synType, int pos, Value value);
};