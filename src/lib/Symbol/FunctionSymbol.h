/*
 * Copyright (c) 2026 F. Lotfi All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#pragma once

#include "VariableSymbol.h"
#include <memory>
#include <string>
#include <vector>

class BlockStatementSyntaxNode;

class FunctionSymbol {
public:
  FunctionSymbol(const std::string &name,
                 std::vector<std::shared_ptr<VariableSymbol>> parameters,
                 const BlockStatementSyntaxNode *body);
  const std::string &Name() const;
  const std::vector<std::shared_ptr<VariableSymbol>> &Parameters() const;
  const BlockStatementSyntaxNode *Body() const;
  static std::shared_ptr<FunctionSymbol> failSymbol() {
    static auto fail = std::make_shared<FunctionSymbol>(
        "unknown", std::vector<std::shared_ptr<VariableSymbol>>(), nullptr);
    return fail;
  }

private:
  std::string mName;
  std::vector<std::shared_ptr<VariableSymbol>> mParameters;
  const BlockStatementSyntaxNode *mBody;
};
