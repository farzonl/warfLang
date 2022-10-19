/*
 * Copyright (c) 2022 F. Lotfi All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#pragma once

#include "BoundStatement.h"

class BoundExpressionStatement : public BoundStatement {
  std::unique_ptr<BoundExpressionNode> mExpression;

public:
  BoundExpressionStatement(std::unique_ptr<BoundExpressionNode> expression);
  BoundExpressionNode *Expression();
  virtual BoundNodeType NodeType() override;
};