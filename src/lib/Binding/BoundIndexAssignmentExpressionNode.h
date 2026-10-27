/*
 * Copyright (c) 2026 F. Lotfi All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#pragma once

#include "BoundExpressionNode.h"
#include <memory>

class BoundIndexAssignmentExpressionNode : public BoundExpressionNode {
public:
  BoundIndexAssignmentExpressionNode(
      std::unique_ptr<BoundExpressionNode> target,
      std::unique_ptr<BoundExpressionNode> index,
      std::unique_ptr<BoundExpressionNode> value);
  virtual ~BoundIndexAssignmentExpressionNode() {}
  virtual BoundNodeKind Kind() override;
  virtual Value::Type Type() override;
  BoundExpressionNode *Target() const;
  BoundExpressionNode *Index() const;
  BoundExpressionNode *ValueExpression() const;

private:
  std::unique_ptr<BoundExpressionNode> mTarget;
  std::unique_ptr<BoundExpressionNode> mIndex;
  std::unique_ptr<BoundExpressionNode> mValue;
};
