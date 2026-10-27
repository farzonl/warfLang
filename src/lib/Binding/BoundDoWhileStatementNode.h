/*
 * Copyright (c) 2026 F. Lotfi All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#pragma once

#include "BoundExpressionNode.h"
#include <memory>

class BoundDoWhileStatementNode : public BoundStatementNode {
public:
  BoundDoWhileStatementNode(std::unique_ptr<BoundStatementNode> body,
                            std::unique_ptr<BoundExpressionNode> condition);

  BoundNodeKind Kind() override;
  const BoundStatementNode *Body() const;
  const BoundExpressionNode *Condition() const;

private:
  std::unique_ptr<BoundStatementNode> mBody;
  std::unique_ptr<BoundExpressionNode> mCondition;
};
