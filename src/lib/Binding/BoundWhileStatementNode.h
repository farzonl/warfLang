/*
 * Copyright (c) 2026 F. Lotfi All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#pragma once

#include "BoundExpressionNode.h"
#include <memory>

class BoundWhileStatementNode : public BoundStatementNode {
public:
  BoundWhileStatementNode(std::unique_ptr<BoundExpressionNode> condition,
                          std::unique_ptr<BoundStatementNode> body);

  BoundNodeKind Kind() override;
  const BoundExpressionNode *Condition() const;
  const BoundStatementNode *Body() const;

private:
  std::unique_ptr<BoundExpressionNode> mCondition;
  std::unique_ptr<BoundStatementNode> mBody;
};
