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

#include "Syntax/SyntaxType.h"

enum class Type { Unknown, Number, Boolean };

class Value {
  union uValue {
    bool boolean;
    int32_t integer;
  };
  Type type;
  uValue val;

public:
  Value() : type(Type::Unknown), val() {}

  Value &operator=(int32_t i) {
    val.integer = i;
    type = Type::Number;
    return *this;
  }
  Value &operator=(bool b) {
    val.boolean = b;
    type = Type::Boolean;
    return *this;
  }

  Value operator+(const Value &v) {
    Value value;
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
    value = -this->asInt();
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
    if(v.asInt() == 0) {
      throw std::runtime_error("Logial error: Divide by zero.");
    }
    Value value;
    value = this->asInt() / v.asInt();
    return value;
  }

  Value operator==(const Value &v) {
    Value value;
    value.type = Type::Boolean;
    if (this->isInt() && v.isInt()) {
      value.val.boolean = this->val.integer == v.val.integer;
    } else if (this->isBool() && v.isBool()) {
      value.val.boolean = this->val.boolean == v.val.boolean;
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

  Type GetType() { return type; }

  int asInt() const {
    assert(type == Type::Number);
    return val.integer;
  }
  bool asBool() const {
    assert(type == Type::Boolean);
    return val.boolean;
  }
  bool isInt() const { return type == Type::Number; }
  bool isBool() const { return type == Type::Boolean; }
  friend class SyntaxToken;
  friend std::ostream &operator<<(std::ostream &out, const Value v);
};

inline std::ostream &operator<<(std::ostream &out, const Value v) {
  switch (v.type) {
  case Type::Boolean:
    out << boolToNameMap.at(v.asBool());
    break;
  case Type::Number:
    out << v.asInt();
    break;
  default:
    throw std::runtime_error("Unsupported Literal Value.");
  }
  return out;
}

inline std::ostream &operator<<(std::ostream &out, const Type t) {
  switch (t) {
  case Type::Boolean:
    out << "Boolean";
    break;
  case Type::Number:
    out << "Number";
    break;
  default:
    throw std::runtime_error("Unsupported Literal type.");
    ;
  }
  return out;
}