/*
 * Copyright (c) 2022 F. Lotfi All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#pragma once

#include "BoundExpressionNode.h"
#include "Symbol/VariableSymbol.h"


class BoundExpressionStatementNode  : public BoundStatementNode {
  public:
    BoundExpressionStatementNode(std::unique_ptr<BoundExpressionNode> expression);
  
    virtual BoundNodeKind Kind() override;
    const BoundExpressionNode* Expression() const;
  private:
    std::unique_ptr<BoundExpressionNode> mExpression;
};