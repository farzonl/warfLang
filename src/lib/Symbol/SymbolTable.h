/*
 * Copyright (c) 2022 F. Lotfi All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#pragma once

#include "Scope.h"
#include "VariableSymbol.h"
#include <string>

#include <unordered_map>

class SymbolTable {
private:
  std::unordered_map<std::string, std::shared_ptr<Scope>> head;

public:
  SymbolTable() = default;
  bool insert(std::shared_ptr<Scope> scope);
  bool insert(std::shared_ptr<VariableSymbol>, std::string scopeName);

  std::shared_ptr<VariableSymbol> find(std::string variableName,
                                       std::string scopeName = "");

  // bool deleteRecord(std::string scopeId, std::string variableName);

  bool modify(std::shared_ptr<VariableSymbol> variable, std::string scopeName);

  friend class SymbolTableMgr;
};