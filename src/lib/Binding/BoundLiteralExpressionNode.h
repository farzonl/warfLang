/*
 * Copyright (c) 2022 F. Lotfi All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#pragma once

#include <memory>

#include "BoundExpressionNode.h"
#include "Syntax/LiteralExpressionNode.h"
#include "Syntax/SyntaxType.h"

class BoundLiteralExpressionNode : public BoundExpressionNode {
public:
  BoundLiteralExpressionNode(LiteralExpressionNode *literal);
  BoundLiteralExpressionNode(int32_t v);
  // Value GetValue();
  virtual BoundNodeType NodeType() override;
  virtual Type GetType() override;
  Value GetValue();

private:
  Value mValue;
};