/*
 * Copyright (c) 2022 F. Lotfi All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#pragma once

#include <memory>

#include "BoundExpressionNode.h"
#include "Symbol/VariableSymbol.h"

class BoundAssignmentExpressionNode : public BoundExpressionNode {
public:
  BoundAssignmentExpressionNode(
      std::shared_ptr<VariableSymbol> variable,
      std::unique_ptr<BoundExpressionNode> boundExpression);

  virtual BoundNodeType NodeType() override;
  BoundExpressionNode *BoundExpression();
  std::string Identifier();
  virtual Type GetType() override;
  std::shared_ptr<VariableSymbol> Variable();

private:
  std::unique_ptr<BoundExpressionNode> mBoundExpression;
 std::shared_ptr<VariableSymbol> mVariable;
};
