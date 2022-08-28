/*
 * Copyright (c) 2022 F. Lotfi All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#pragma once

#include "Binding/ValueType.h"
#include <string>

class Scope;

class VariableSymbol {
public:
  VariableSymbol(const std::string &name, Type type);
  const std::string &Name() const;
  Type GetType() const;
  Value GetValue() const;
  void SetValue(Value v);
  static std::shared_ptr<VariableSymbol> failSymbol() {
    static auto fail =
        std::make_shared<VariableSymbol>("unknown", Type::Unknown);
    return fail;
  }
  std::string GetScopeName();
  /*bool operator==(const VariableSymbol &variable) const {
    return (this->mName == variable.mName);
  }
  bool operator==(const std::shared_ptr<VariableSymbol> variable) const {
    return (this->mName == variable->mName);
  }
  class Hash {
  public:
    size_t operator()(const VariableSymbol &variable) const;
    size_t operator()(const std::shared_ptr<VariableSymbol> variable) const;
  };*/
friend class Scope;
private:
  std::string mName;
  Type mType;
  Scope* mScope;
  Value mValue;
};