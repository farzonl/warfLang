/*
 * Copyright (c) 2022 F. Lotfi All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#pragma once

#include "ValueType.h"

enum class BoundNodeKind {
  Unknown,
  // Expressions
  AssignmentExpression,
  BinaryExpression,
  UnaryExpression,
  LiteralExpression,

  // Statements
  BlockStatement,
  VariableDeclaration,
  ExpressionStatement,
};

class BoundNode {
public:
  BoundNode() = default;
  virtual BoundNodeKind Kind() = 0;
  virtual ~BoundNode() {}
};

class BoundExpressionNode : public BoundNode {
public:
  BoundExpressionNode() = default;
  virtual Value::Type Type() = 0;
  virtual ~BoundExpressionNode() {}
};

class BoundStatementNode : public BoundNode {};