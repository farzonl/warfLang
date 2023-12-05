// Copyright (c) 2022 F. Lotfi All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "SymbolTableMgr.h"
#include "SymbolTable.h"
#include <limits>

SymbolTable SymbolTableMgr::symbolTable;

void SymbolTableMgr::init() {
  if (symbolTable.head.size() == 0) {
    std::shared_ptr<TextSpan> globalTextSpan =
        std::make_shared<TextSpan>(0, std::numeric_limits<int32_t>::max());
    std::shared_ptr<Scope> globalScope =
        std::make_shared<Scope>(Scope::ScopeKind::Global, globalTextSpan);
    symbolTable.insert(globalScope);
  }
}
std::shared_ptr<Scope> SymbolTableMgr::getGlobalScope() {
  const std::string &global =
      Scope::ScopeKindToName.at(Scope::ScopeKind::Global);
  auto iter = symbolTable.head.find(global);
  if (iter != symbolTable.head.end()) {
    return iter->second;
  }
  return nullptr;
}
std::shared_ptr<VariableSymbol> SymbolTableMgr::find(std::string variableName,
                                                     std::string scopeName) {
  return symbolTable.find(variableName, scopeName);
}

bool SymbolTableMgr::modify(std::shared_ptr<VariableSymbol> variable,
                            std::string scopeName) {
  return symbolTable.modify(variable, scopeName);
}

bool SymbolTableMgr::insert(std::shared_ptr<VariableSymbol> variable,
                            std::string scopeName) {
  if (scopeName == "") {
    scopeName = Scope::ScopeKindToName.at(Scope::ScopeKind::Global);
  }
  return symbolTable.insert(variable, scopeName);
}