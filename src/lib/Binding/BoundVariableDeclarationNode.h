/*
 * Copyright (c) 2022 F. Lotfi All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#pragma once

#include "BoundExpressionNode.h"
#include "Symbol/VariableSymbol.h"

class BoundVariableDeclarationNode : public BoundStatementNode {
public:
  BoundVariableDeclarationNode(
      std::shared_ptr<VariableSymbol> variable,
      std::unique_ptr<BoundExpressionNode> initializer);

  virtual BoundNodeKind Kind() override;

  std::shared_ptr<VariableSymbol> Variable() const;
  const BoundExpressionNode *Initializer() const;

private:
  std::shared_ptr<VariableSymbol> mVariable;
  std::unique_ptr<BoundExpressionNode> mInitializer;
};