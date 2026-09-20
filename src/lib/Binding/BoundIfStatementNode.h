/*
 * Copyright (c) 2026 F. Lotfi All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#pragma once

#include "BoundExpressionNode.h"
#include <memory>

class BoundIfStatementNode : public BoundStatementNode {
public:
  BoundIfStatementNode(std::unique_ptr<BoundExpressionNode> condition,
                       std::unique_ptr<BoundStatementNode> thenStatement,
                       std::unique_ptr<BoundStatementNode> elseStatement);

  BoundNodeKind Kind() override;
  const BoundExpressionNode *Condition() const;
  const BoundStatementNode *ThenStatement() const;
  const BoundStatementNode *ElseStatement() const;

private:
  std::unique_ptr<BoundExpressionNode> mCondition;
  std::unique_ptr<BoundStatementNode> mThenStatement;
  std::unique_ptr<BoundStatementNode> mElseStatement;
};
