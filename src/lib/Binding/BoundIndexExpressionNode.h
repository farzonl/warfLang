/*
 * Copyright (c) 2026 F. Lotfi All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#pragma once

#include "BoundExpressionNode.h"
#include <memory>

class BoundIndexExpressionNode : public BoundExpressionNode {
public:
  BoundIndexExpressionNode(std::unique_ptr<BoundExpressionNode> target,
                           std::unique_ptr<BoundExpressionNode> index,
                           Value::Type elementType);
  virtual ~BoundIndexExpressionNode() {}
  virtual BoundNodeKind Kind() override;
  virtual Value::Type Type() override;
  BoundExpressionNode *Target() const;
  BoundExpressionNode *Index() const;

private:
  std::unique_ptr<BoundExpressionNode> mTarget;
  std::unique_ptr<BoundExpressionNode> mIndex;
  Value::Type mElementType;
};
