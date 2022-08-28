/*
 * Copyright (c) 2022 F. Lotfi All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#pragma once

#include "SymbolTable.h"
#include <memory>

class SymbolTableMgr {
public:
  static void init();
  static std::shared_ptr<Scope> getGlobalScope();
  static std::shared_ptr<VariableSymbol> find(std::string variableName,
                                              std::string scopeName = "");
  static bool modify(std::shared_ptr<VariableSymbol> variable, std::string scopeName);
  static bool insert(std::shared_ptr<VariableSymbol> variable, std::string scopeName = "");
private:
  static SymbolTable symbolTable;
  SymbolTableMgr() = delete;
};