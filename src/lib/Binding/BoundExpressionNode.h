/*
 * Copyright (c) 2022 F. Lotfi All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#pragma once

#include "ValueType.h"

enum class BoundNodeType { Unknown, UnaryExpression, LiteralExpression };

class BoundExpressionNode {
public:
  BoundExpressionNode() = default;
  virtual BoundNodeType NodeType() = 0;
  virtual Type GetType() = 0;
  virtual ~BoundExpressionNode() {}
};