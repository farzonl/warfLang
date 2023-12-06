/*
 * Copyright (c) 2022 F. Lotfi All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#pragma once

#include <memory>

#include "BoundExpressionNode.h"
#include "Syntax/LiteralExpressionNode.h"
#include "Syntax/SyntaxKind.h"

class BoundLiteralExpressionNode : public BoundExpressionNode {
public:
  BoundLiteralExpressionNode(LiteralExpressionNode *literal);
  BoundLiteralExpressionNode(int32_t v);
  virtual ~BoundLiteralExpressionNode() {}
  // Value GetValue();
  virtual BoundNodeKind NodeKind() override;
  virtual Value::Type Type() override;
  Value GetValue();

private:
  Value mValue;
};