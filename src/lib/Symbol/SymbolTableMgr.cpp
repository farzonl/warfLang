// Copyright (c) 2022 F. Lotfi All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "SymbolTableMgr.h"
#include "SymbolTable.h"

SymbolTable SymbolTableMgr::symbolTable;

void SymbolTableMgr::init() {
  std::shared_ptr<TextSpan> globalTextSpan =
      std::make_shared<TextSpan>(0, INT_MAX);
  std::shared_ptr<Scope> globalScope =
      std::make_shared<Scope>(Scope::ScopeType::Global, globalTextSpan);
  symbolTable.insert(globalScope);
}
std::shared_ptr<Scope> SymbolTableMgr::getGlobalScope() {
  const std::string& global = Scope::ScopeTypeToName.at(Scope::ScopeType::Global);
  auto iter =
      symbolTable.head.find(global);
  if (iter != symbolTable.head.end()) {
    return iter->second;
  }
  return nullptr;
}
std::shared_ptr<VariableSymbol> SymbolTableMgr::find(std::string variableName,
                                              std::string scopeName) {
    return symbolTable.find(variableName, scopeName);
}

bool SymbolTableMgr::modify(std::shared_ptr<VariableSymbol> variable, std::string scopeName) {
    return symbolTable.modify(variable, scopeName);
}

bool SymbolTableMgr::insert(std::shared_ptr<VariableSymbol> variable, std::string scopeName) {
    if(scopeName == "") {
        scopeName = Scope::ScopeTypeToName.at(Scope::ScopeType::Global);
    }
    return symbolTable.insert(variable, scopeName);
}