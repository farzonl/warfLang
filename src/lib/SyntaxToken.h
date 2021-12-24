#pragma once
#include <cassert>
#include <iostream>
#include <string>
#include <unordered_map>

#include "SyntaxNode.h"
#include "SyntaxType.h"

enum class Type { UnknownType, IntegerType, BooleanType };

enum class Errors { LexError, ParseError };

static const std::unordered_map<bool, std::string> boolToNameMap = {
    {true, "true"},
    {false, "false"},
};

static const std::unordered_map<std::string, bool> boolStrToValueMap = {
    {"true", true},
    {"false", false},
};

class ValueType {
  union Value {
    bool boolean;
    int32_t integer;
  };
  Type type;
  Value val;

public:
  ValueType() : type(Type::UnknownType), val() {}
  ValueType &operator=(int32_t i) {
    val.integer = i;
    type = Type::IntegerType;
    return *this;
  }
  ValueType &operator=(bool b) {
    val.boolean = b;
    type = Type::BooleanType;
    return *this;
  }
  int asInt() const {
    assert(type == Type::IntegerType);
    return val.integer;
  }
  bool asBool() const {
    assert(type == Type::BooleanType);
    return val.boolean;
  }
  bool isInt() const { return type == Type::IntegerType; }
  bool isBool() const { return type == Type::BooleanType; }
  friend class SyntaxToken;
  friend std::ostream &operator<<(std::ostream &out, const ValueType v);
};

inline std::ostream &operator<<(std::ostream &out, const ValueType v) {
  switch (v.type) {
  case Type::BooleanType:
    out << boolToNameMap.at(v.asBool());
    break;
  case Type::IntegerType:
    out << v.asInt();
    break;
  default:
    std::cerr << "Literal type is Unknown or not supported." << std::endl;
    throw;
  }
  return out;
}

class SyntaxToken : public SyntaxNode {
private:
  int mPosition;
  std::string mText;
  ValueType mValue;

public:
  bool HasValue() const;
  const ValueType &Value() const;
  int Position() const;
  std::string Text() const;
  SyntaxToken(SyntaxType synType, int pos, std::string text);
  SyntaxToken(SyntaxType synType, int pos, ValueType value);
};