// Copyright (c) 2022 F. Lotfi All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "Scope.h"
#include <functional>

const std::unordered_map<std::string, Scope::ScopeKind> Scope::NameToScopeKind =
    {
        {".global", ScopeKind::Global},
};

const std::unordered_map<Scope::ScopeKind, std::string> Scope::ScopeKindToName =
    {
        {ScopeKind::Global, ".global"},
};

Scope::Scope(ScopeKind scopeKind, std::shared_ptr<TextSpan> textSpan,
             std::string scopeName, std::shared_ptr<Scope> parent)
    : mScopeKind(scopeKind), mTextspan(textSpan),
      mName(ScopeKindToName.at(scopeKind) + scopeName), mParent(parent),
      mVariables(), mFunctions() {}

const std::string &Scope::Name() const { return mName; }

std::shared_ptr<TextSpan> Scope::GetTextSpan() { return mTextspan; }

void Scope::insert(std::shared_ptr<VariableSymbol> variable) {
  variable->mScope = this;
  mVariables[variable->Name()] = variable;
}

void Scope::insert(std::shared_ptr<FunctionSymbol> function) {
  mFunctions[function->Name()] = function;
}

std::shared_ptr<VariableSymbol>
Scope::lookupLocal(const std::string &name) const {
  auto variable = mVariables.find(name);
  return variable == mVariables.end() ? VariableSymbol::failSymbol()
                                      : variable->second;
}

std::shared_ptr<VariableSymbol> Scope::lookup(const std::string &name) const {
  auto variable = lookupLocal(name);
  if (variable != VariableSymbol::failSymbol()) {
    return variable;
  }
  return mParent == nullptr ? VariableSymbol::failSymbol()
                            : mParent->lookup(name);
}

std::shared_ptr<FunctionSymbol>
Scope::lookupFunctionLocal(const std::string &name) const {
  auto function = mFunctions.find(name);
  return function == mFunctions.end() ? FunctionSymbol::failSymbol()
                                      : function->second;
}

std::shared_ptr<FunctionSymbol>
Scope::lookupFunction(const std::string &name) const {
  auto function = lookupFunctionLocal(name);
  if (function != FunctionSymbol::failSymbol()) {
    return function;
  }
  return mParent == nullptr ? FunctionSymbol::failSymbol()
                            : mParent->lookupFunction(name);
}

std::shared_ptr<Scope> Scope::Parent() const { return mParent; }

/*size_t Scope::Hash::operator()(const Scope &scope) const {
  return std::hash<std::string>{}(scope.Name());
}

size_t Scope::Hash::operator()(const std::shared_ptr<Scope> scope) const {
  return std::hash<std::string>{}(scope->Name());
}*/
