/*
 * Copyright (c) 2022 F. Lotfi All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#pragma once

#include "FunctionSymbol.h"
#include "TextSpan.h"
#include "VariableSymbol.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>

class Scope {
public:
  enum class ScopeKind { Global };

private:
  ScopeKind mScopeKind;
  std::shared_ptr<TextSpan> mTextspan;
  std::string mName;
  std::shared_ptr<Scope> mParent;
  std::unordered_map<std::string, std::shared_ptr<VariableSymbol>> mVariables;
  std::unordered_map<std::string, std::shared_ptr<FunctionSymbol>> mFunctions;
  Scope() = default;

public:
  Scope(ScopeKind scopeKind, std::shared_ptr<TextSpan> textSpan,
        std::string scopeName = "", std::shared_ptr<Scope> parent = nullptr);

  const std::string &Name() const;
  void insert(std::shared_ptr<VariableSymbol> variable);
  void insert(std::shared_ptr<FunctionSymbol> function);
  std::shared_ptr<VariableSymbol> lookupLocal(const std::string &name) const;
  std::shared_ptr<VariableSymbol> lookup(const std::string &name) const;
  std::shared_ptr<FunctionSymbol>
  lookupFunctionLocal(const std::string &name) const;
  std::shared_ptr<FunctionSymbol> lookupFunction(const std::string &name) const;
  std::shared_ptr<Scope> Parent() const;

  std::shared_ptr<TextSpan> GetTextSpan();
  /*bool operator==(const Scope &scope) const {
    return (this->mName == scope.mName);
  }
  bool operator==(const std::shared_ptr<Scope> scope) const {
    return (this->mName == scope->mName);
  }
  class Hash {
  public:
    size_t operator()(const Scope &scope) const;
    size_t operator()(const std::shared_ptr<Scope> scope) const;
  };*/

  static const std::unordered_map<std::string, ScopeKind> NameToScopeKind;
  static const std::unordered_map<ScopeKind, std::string> ScopeKindToName;

  friend class SymbolTable;
};
