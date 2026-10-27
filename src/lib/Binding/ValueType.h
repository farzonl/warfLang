/*
 * Copyright (c) 2022 F. Lotfi All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#pragma once

#include <cassert>
#include <cstdint>
#include <iostream>
#include <limits>
#include <string>
#include <unordered_map>

#include "Syntax/SyntaxKind.h"

class Value {
public:
  enum class Type { Unknown, Number, Boolean, String };

private:
  union uValue {
    bool boolean;
    int32_t integer;
  };
  Type type;
  uValue val;
  std::string str;

public:
  Value() : type(Type::Unknown), val(), str() {}
  Value(bool b) : type(Type::Boolean), val(), str() { val.boolean = b; }
  Value(int32_t i) : type(Type::Number), val(), str() { val.integer = i; }
  Value(std::string s) : type(Type::String), val(), str(std::move(s)) {}
  explicit Value(const char *s) : type(Type::String), val(), str(s) {}

  Value &operator=(int32_t i) {
    val.integer = i;
    str.clear();
    type = Type::Number;
    return *this;
  }
  Value &operator=(bool b) {
    val.boolean = b;
    str.clear();
    type = Type::Boolean;
    return *this;
  }
  Value &operator=(std::string s) {
    str = std::move(s);
    type = Type::String;
    return *this;
  }
  Value &operator=(const char *s) {
    str = s;
    type = Type::String;
    return *this;
  }

  Value operator+(const Value &v) {
    Value value;
    if (this->isString() && v.isString()) {
      value = this->asString() + v.asString();
      return value;
    }
    value = this->asInt() + v.asInt();
    return value;
  }

  Value operator-(const Value &v) {
    Value value;
    value = this->asInt() - v.asInt();
    return value;
  }

  Value operator-() {
    Value value;
    value = static_cast<int32_t>(-static_cast<int64_t>(this->asInt()));
    return value;
  }

  Value operator~() {
    Value value;
    value = ~this->asInt();
    return value;
  }

  Value operator|(const Value &v) {
    Value value;
    value = this->asInt() | v.asInt();
    return value;
  }

  Value operator&(const Value &v) {
    Value value;
    value = this->asInt() & v.asInt();
    return value;
  }

  Value operator^(const Value &v) {
    Value value;
    value = this->asInt() ^ v.asInt();
    return value;
  }

  Value operator*(const Value &v) {
    Value value;
    value = this->asInt() * v.asInt();
    return value;
  }

  Value operator<(const Value &v) {
    Value value;
    value = this->asInt() < v.asInt();
    return value;
  }

  Value operator>(const Value &v) {
    Value value;
    value = this->asInt() > v.asInt();
    return value;
  }

  Value operator<=(const Value &v) {
    Value value;
    value = this->asInt() <= v.asInt();
    return value;
  }

  Value operator>=(const Value &v) {
    Value value;
    value = this->asInt() >= v.asInt();
    return value;
  }

  Value operator/(const Value &v) {
    if (v.asInt() == 0) {
      throw std::runtime_error("Logial error: Divide by zero.");
    }
    Value value;
    value = this->asInt() / v.asInt();
    return value;
  }

  Value operator%(const Value &v) {
    if (v.asInt() == 0) {
      throw std::runtime_error("Logial error: Divide by zero.");
    }
    if (this->asInt() == std::numeric_limits<int32_t>::min() &&
        v.asInt() == -1) {
      Value value;
      value = 0;
      return value;
    }
    Value value;
    value = this->asInt() % v.asInt();
    return value;
  }

  Value operator==(const Value &v) {
    Value value;
    value.type = Type::Boolean;
    if (this->isInt() && v.isInt()) {
      value.val.boolean = this->val.integer == v.val.integer;
    } else if (this->isBool() && v.isBool()) {
      value.val.boolean = this->val.boolean == v.val.boolean;
    } else if (this->isString() && v.isString()) {
      value.val.boolean = this->str == v.str;
    } else {
      value.val.boolean = false;
    }
    return value;
  }

  Value operator!=(const Value &v) {
    Value value;
    value.type = Type::Boolean;
    if (this->isInt() && v.isInt()) {
      value.val.boolean = this->val.integer != v.val.integer;
    } else if (this->isBool() && v.isBool()) {
      value.val.boolean = this->val.boolean != v.val.boolean;
    } else if (this->isString() && v.isString()) {
      value.val.boolean = this->str != v.str;
    } else {
      value.val.boolean = false;
    }
    return value;
  }

  Value operator&&(const Value &v) {
    Value value;
    value.type = Type::Boolean;
    value.val.boolean = this->asBool() && v.asBool();
    return value;
  }

  Value operator||(const Value &v) {
    Value value;
    value.type = Type::Boolean;
    value.val.boolean = this->asBool() || v.asBool();
    return value;
  }

  Value operator!() {
    Value value;
    value.type = Type::Boolean;
    value.val.boolean = !this->asBool();
    return value;
  }

  Type VType() { return type; }

  int asInt() const {
    assert(type == Type::Number);
    return val.integer;
  }
  bool asBool() const {
    assert(type == Type::Boolean);
    return val.boolean;
  }
  const std::string &asString() const {
    assert(type == Type::String);
    return str;
  }
  bool isInt() const { return type == Type::Number; }
  bool isBool() const { return type == Type::Boolean; }
  bool isString() const { return type == Type::String; }
  friend class SyntaxToken;
  friend std::ostream &operator<<(std::ostream &out, const Value v);
};

inline std::ostream &operator<<(std::ostream &out, const Value v) {
  switch (v.type) {
  case Value::Type::Boolean:
    out << boolToNameMap.at(v.asBool());
    break;
  case Value::Type::Number:
    out << v.asInt();
    break;
  case Value::Type::String:
    out << v.asString();
    break;
  default:
    throw std::runtime_error("Unsupported Literal Value.");
  }
  return out;
}

inline std::ostream &operator<<(std::ostream &out, const Value::Type t) {
  switch (t) {
  case Value::Type::Boolean:
    out << "Boolean";
    break;
  case Value::Type::Number:
    out << "Number";
    break;
  case Value::Type::String:
    out << "String";
    break;
  default:
    throw std::runtime_error("Unsupported Literal type.");
  }
  return out;
}
