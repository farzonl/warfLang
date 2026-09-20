/*
 * Copyright (c) 2026 F. Lotfi All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#pragma once

#include "BoundExpressionNode.h"
#include "Symbol/FunctionSymbol.h"
#include <memory>

class BoundFunctionDeclarationNode : public BoundStatementNode {
public:
  BoundFunctionDeclarationNode(std::shared_ptr<FunctionSymbol> function,
                               std::unique_ptr<BoundStatementNode> body);
  virtual BoundNodeKind Kind() override;
  std::shared_ptr<FunctionSymbol> Function() const;
  const BoundStatementNode *Body() const;

private:
  std::shared_ptr<FunctionSymbol> mFunction;
  std::unique_ptr<BoundStatementNode> mBody;
};