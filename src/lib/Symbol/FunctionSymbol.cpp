// Copyright (c) 2026 F. Lotfi All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "FunctionSymbol.h"

FunctionSymbol::FunctionSymbol(
    const std::string &name,
    std::vector<std::shared_ptr<VariableSymbol>> parameters)
    : mName(name), mParameters(std::move(parameters)) {}

const std::string &FunctionSymbol::Name() const { return mName; }

const std::vector<std::shared_ptr<VariableSymbol>> &
FunctionSymbol::Parameters() const {
  return mParameters;
}
