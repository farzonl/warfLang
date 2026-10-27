/*
 * Copyright (c) 2026 F. Lotfi All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#pragma once

#include "BoundExpressionNode.h"
#include <memory>
#include <vector>

class BoundArrayLiteralExpressionNode : public BoundExpressionNode {
public:
  BoundArrayLiteralExpressionNode(
      std::vector<std::unique_ptr<BoundExpressionNode>> elements,
      Value::Type elementType);
  virtual ~BoundArrayLiteralExpressionNode() {}
  virtual BoundNodeKind Kind() override;
  virtual Value::Type Type() override;
  virtual Value::Type ElementType() const override { return mElementType; }
  const std::vector<std::unique_ptr<BoundExpressionNode>> &Elements() const;

private:
  std::vector<std::unique_ptr<BoundExpressionNode>> mElements;
  Value::Type mElementType;
};
