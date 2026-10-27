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
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "Syntax/SyntaxKind.h"

class Value {
public:
  enum class Type { Unknown, Number, Boolean, Float, String, Array };

private:
  union uValue {
    bool boolean;
    int32_t integer;
    double floating;
  };
  Type type;
  uValue val;
  std::string str;
  std::shared_ptr<std::vector<Value>> array;

public:
  Value() : type(Type::Unknown), val(), str(), array(nullptr) {}
  Value(bool b) : type(Type::Boolean), val(), str(), array(nullptr) {
    val.boolean = b;
  }
  Value(int32_t i) : type(Type::Number), val(), str(), array(nullptr) {
    val.integer = i;
  }
  Value(double d) : type(Type::Float), val(), str(), array(nullptr) {
    val.floating = d;
  }
  Value(std::string s)
      : type(Type::String), val(), str(std::move(s)), array(nullptr) {}
  explicit Value(const char *s)
      : type(Type::String), val(), str(s), array(nullptr) {}

  Value &operator=(int32_t i) {
    val.integer = i;
    str.clear();
    array.reset();
    type = Type::Number;
    return *this;
  }
  Value &operator=(double d) {
    val.floating = d;
    str.clear();
    array.reset();
    type = Type::Float;
    return *this;
  }
  Value &operator=(bool b) {
    val.boolean = b;
    str.clear();
    array.reset();
    type = Type::Boolean;
    return *this;
  }
  Value &operator=(std::string s) {
    str = std::move(s);
    array.reset();
    type = Type::String;
    return *this;
  }
  Value &operator=(const char *s) {
    str = s;
    array.reset();
    type = Type::String;
    return *this;
  }
  Value &operator=(std::shared_ptr<std::vector<Value>> arr) {
    str.clear();
    array = std::move(arr);
    type = Type::Array;
    return *this;
  }

  Value operator+(const Value &v) {
    Value value;
    if (this->isString() && v.isString()) {
      value = this->asString() + v.asString();
      return value;
    }
    if (this->isFloat() && v.isFloat()) {
      value = this->asFloat() + v.asFloat();
      return value;
    }
    value = this->asInt() + v.asInt();
    return value;
  }

  Value operator-(const Value &v) {
    Value value;
    if (this->isFloat() && v.isFloat()) {
      value = this->asFloat() - v.asFloat();
      return value;
    }
    value = this->asInt() - v.asInt();
    return value;
  }

  Value operator-() {
    Value value;
    if (this->isFloat()) {
      value = -this->asFloat();
      return value;
    }
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
    if (this->isFloat() && v.isFloat()) {
      value = this->asFloat() * v.asFloat();
      return value;
    }
    value = this->asInt() * v.asInt();
    return value;
  }

  Value operator<(const Value &v) {
    Value value;
    if (this->isFloat() && v.isFloat()) {
      value = this->asFloat() < v.asFloat();
      return value;
    }
    value = this->asInt() < v.asInt();
    return value;
  }

  Value operator>(const Value &v) {
    Value value;
    if (this->isFloat() && v.isFloat()) {
      value = this->asFloat() > v.asFloat();
      return value;
    }
    value = this->asInt() > v.asInt();
    return value;
  }

  Value operator<=(const Value &v) {
    Value value;
    if (this->isFloat() && v.isFloat()) {
      value = this->asFloat() <= v.asFloat();
      return value;
    }
    value = this->asInt() <= v.asInt();
    return value;
  }

  Value operator>=(const Value &v) {
    Value value;
    if (this->isFloat() && v.isFloat()) {
      value = this->asFloat() >= v.asFloat();
      return value;
    }
    value = this->asInt() >= v.asInt();
    return value;
  }

  Value operator/(const Value &v) {
    if (this->isFloat() && v.isFloat()) {
      Value value;
      value = this->asFloat() / v.asFloat();
      return value;
    }
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
    } else if (this->isFloat() && v.isFloat()) {
      value.val.boolean = this->val.floating == v.val.floating;
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
    } else if (this->isFloat() && v.isFloat()) {
      value.val.boolean = this->val.floating != v.val.floating;
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

  Type VType() const { return type; }

  int asInt() const {
    assert(type == Type::Number);
    return val.integer;
  }
  double asFloat() const {
    assert(type == Type::Float);
    return val.floating;
  }
  bool asBool() const {
    assert(type == Type::Boolean);
    return val.boolean;
  }
  const std::string &asString() const {
    assert(type == Type::String);
    return str;
  }
  const std::shared_ptr<std::vector<Value>> &asArray() const {
    assert(type == Type::Array);
    return array;
  }
  bool isInt() const { return type == Type::Number; }
  bool isFloat() const { return type == Type::Float; }
  bool isBool() const { return type == Type::Boolean; }
  bool isString() const { return type == Type::String; }
  bool isArray() const { return type == Type::Array; }
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
  case Value::Type::Float:
    out << v.asFloat();
    break;
  case Value::Type::String:
    out << v.asString();
    break;
  case Value::Type::Array:
    out << "[";
    if (v.array) {
      for (size_t i = 0; i < v.array->size(); ++i) {
        if (i > 0) {
          out << ", ";
        }
        out << (*v.array)[i];
      }
    }
    out << "]";
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
  case Value::Type::Float:
    out << "Float";
    break;
  case Value::Type::String:
    out << "String";
    break;
  case Value::Type::Array:
    out << "Array";
    break;
  default:
    throw std::runtime_error("Unsupported Literal type.");
  }
  return out;
}
