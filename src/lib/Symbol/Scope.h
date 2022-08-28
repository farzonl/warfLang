/*
 * Copyright (c) 2022 F. Lotfi All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#pragma once

#include "TextSpan.h"
#include "VariableSymbol.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <unordered_set>

class Scope {
public:
  enum class ScopeType { Global };

private:
  ScopeType mScopeType;
  std::shared_ptr<TextSpan> mTextspan;
  std::string mName;
  std::unordered_map<std::string, std::shared_ptr<VariableSymbol>> mVariables;
  Scope() = default;

public:
  Scope(ScopeType scopeType, std::shared_ptr<TextSpan> textSpan,
        std::string scopeName = "");

  const std::string &Name() const;
  void insert(std::shared_ptr<VariableSymbol> variable);

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

  static const std::unordered_map<std::string, ScopeType> NameToScopeType;
  static const std::unordered_map<ScopeType, std::string> ScopeTypeToName;

  friend class SymbolTable;
};