/*
 * Copyright (c) 2022 F. Lotfi All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#pragma once

#include "ValueType.h"

enum class BoundNodeKind {
  Unknown,
  AssignmentExpression,
  BinaryExpression,
  UnaryExpression,
  LiteralExpression
};

class BoundExpressionNode {
public:
  BoundExpressionNode() = default;
  virtual BoundNodeKind NodeKind() = 0;
  virtual Value::Type Type() = 0;
  virtual ~BoundExpressionNode() {}
};