// Copyright (c) 2022 F. Lotfi All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "Scope.h"
#include <functional>

const std::unordered_map<std::string, Scope::ScopeType>Scope:: NameToScopeType = {
    {".global", ScopeType::Global},
};

const std::unordered_map<Scope::ScopeType, std::string> Scope::ScopeTypeToName = {
    {ScopeType::Global, ".global"},
};

Scope::Scope(ScopeType scopeType, std::shared_ptr<TextSpan> textSpan,
             std::string scopeName)
    : mScopeType(scopeType), mTextspan(textSpan),
      mName(ScopeTypeToName.at(scopeType) + scopeName), mVariables() {}

const std::string &Scope::Name() const { return mName; }

std::shared_ptr<TextSpan> Scope::GetTextSpan() {
    return mTextspan;
}

void Scope::insert(std::shared_ptr<VariableSymbol> variable) {
    variable->mScope = this;
    mVariables[variable->Name()] = variable;
}

/*size_t Scope::Hash::operator()(const Scope &scope) const {
  return std::hash<std::string>{}(scope.Name());
}

size_t Scope::Hash::operator()(const std::shared_ptr<Scope> scope) const {
  return std::hash<std::string>{}(scope->Name());
}*/
