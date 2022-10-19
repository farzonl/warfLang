/*
 * Copyright (c) 2022 F. Lotfi All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#pragma once

#include "ValueType.h"

enum class BoundNodeType {
  Unknown,
  // Statements
  BlockStatement,
  ExpressionStatement,

  // Expressions
  AssignmentExpression,
  BinaryExpression,
  UnaryExpression,
  LiteralExpression
};

class BoundNode {
public:
  BoundNode() = default;
  virtual ~BoundNode() {}
  virtual BoundNodeType NodeType() = 0;
};

class BoundExpressionNode : public BoundNode {
public:
  virtual Type GetType() = 0;
};