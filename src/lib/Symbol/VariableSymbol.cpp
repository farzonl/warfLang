// Copyright (c) 2022 F. Lotfi All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "VariableSymbol.h"
// #include <functional>
#include "Scope.h"

VariableSymbol::VariableSymbol(const std::string &name, bool isReadOnly, Value::Type type)
    : mName(name), mType(type){};

const std::string &VariableSymbol::Name() const { return mName; }
Value::Type VariableSymbol::Type() const { return mType; }

std::string VariableSymbol::GetScopeName() {
  return this->mScope != nullptr ? this->mScope->Name() : "";
}

Value VariableSymbol::GetValue() const { return mValue; }
void VariableSymbol::SetValue(Value v) { mValue = v; }

bool VariableSymbol::IsReadOnly() const { return mIsReadOnly; }

/*size_t VariableSymbol::Hash::operator()(const VariableSymbol &variable) const
{ return std::hash<std::string>{}(variable.Name());
}

size_t VariableSymbol::Hash::operator()(
    const std::shared_ptr<VariableSymbol> variable) const {
  return std::hash<std::string>{}(variable->Name());
}*/