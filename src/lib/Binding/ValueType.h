#include <unordered_map>
#include <string>
#include <cassert>
#include <iostream>

enum class Type { Unknown, Number, Boolean };

static const std::unordered_map<bool, std::string> boolToNameMap = {
    {true, "true"},
    {false, "false"},
};

static const std::unordered_map<std::string, bool> boolStrToValueMap = {
    {"true", true},
    {"false", false},
};

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
    std::cerr << "Literal type is Unknown or not supported." << std::endl;
    throw;
  }
  return out;
}