/*
 * Copyright (c) 2026 F. Lotfi All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#pragma once

#include "BoundExpressionNode.h"
#include <memory>

class BoundReturnStatementNode : public BoundStatementNode {
public:
  BoundReturnStatementNode(std::unique_ptr<BoundExpressionNode> expression);

  BoundNodeKind Kind() override;
  const BoundExpressionNode *Expression() const;

private:
  std::unique_ptr<BoundExpressionNode> mExpression;
};
