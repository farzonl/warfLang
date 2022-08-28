/*
 * Copyright (c) 2022 F. Lotfi All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#include "SymbolTable.h"

bool SymbolTable::insert(std::shared_ptr<Scope> scope) {
  head[scope->Name()] = scope;
  return true;
}

bool SymbolTable::insert(std::shared_ptr<VariableSymbol> variable,
                         std::string scopeName) {
  auto scopeItrator = head.find(scopeName);
  if (scopeItrator != head.end()) {
    scopeItrator->second->insert(variable);
    return true;
  }

  return false;
}

template <class T>
std::shared_ptr<T>
customfind(std::unordered_map<std::string, std::shared_ptr<T>> map,
           std::string key) {
  auto mapIterator = map.find(key);
  if (mapIterator == map.end()) {
    return nullptr;
  }
  return mapIterator->second;
}

std::shared_ptr<VariableSymbol> SymbolTable::find(std::string variableName,
                                                  std::string scopeName) {
  if (scopeName != "") {
    auto scopePtr = customfind<Scope>(this->head, scopeName);
    if (nullptr == scopePtr) {
      return VariableSymbol::failSymbol();
    }
    auto variablePtr =
        customfind<VariableSymbol>(scopePtr->mVariables, variableName);
    if (nullptr == variablePtr) {
      return VariableSymbol::failSymbol();
    }
    return variablePtr;
  }

  for (auto const &[scopeName, scopePtr] : this->head) {
    auto variablePtr =
        customfind<VariableSymbol>(scopePtr->mVariables, variableName);
    if (nullptr != variablePtr) {
      return variablePtr;
    }
  }

  return VariableSymbol::failSymbol();
}

bool SymbolTable::modify(std::shared_ptr<VariableSymbol> variable,
                         std::string scopeName) {
  auto scopePtr = customfind<Scope>(this->head, scopeName);
  if (nullptr == scopePtr) {
    return false;
  }
  scopePtr->insert(variable);
  return true;
}
