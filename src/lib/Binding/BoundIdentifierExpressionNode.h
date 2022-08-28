/*
 * Copyright (c) 2022 F. Lotfi All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#pragma once

#include "BoundExpressionNode.h"
#include "Symbol/VariableSymbol.h"
#include <string>

class BoundIdentifierExpressionNode : public BoundExpressionNode {
public:
  BoundIdentifierExpressionNode(std::shared_ptr<VariableSymbol> variable);

  virtual BoundNodeType NodeType() override;
  virtual Type GetType() override;
  std::string Name() { return mVariable->Name(); }
  std::shared_ptr<VariableSymbol> Variable() { return mVariable; }

private:
  std::shared_ptr<VariableSymbol> mVariable;
};
