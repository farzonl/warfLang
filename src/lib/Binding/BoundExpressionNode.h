/*
 * Copyright (c) 2022 F. Lotfi All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#pragma once

#include "Symbol/TextSpan.h"
#include "ValueType.h"

enum class BoundNodeKind {
  Unknown,
  // Expressions
  AssignmentExpression,
  BinaryExpression,
  UnaryExpression,
  LiteralExpression,
  CallExpression,
  ArrayLiteralExpression,
  IndexExpression,
  IndexAssignmentExpression,

  // Statements
  BlockStatement,
  VariableDeclaration,
  FunctionDeclaration,
  ExpressionStatement,
  IfStatement,
  WhileStatement,
  DoWhileStatement,
  ForStatement,
  BreakStatement,
  ContinueStatement,
  ReturnStatement,
};

class BoundNode {
public:
  BoundNode() = default;
  explicit BoundNode(TextSpan span) : mSpan(span) {}
  virtual BoundNodeKind Kind() = 0;
  virtual ~BoundNode() {}
  const TextSpan &Span() const { return mSpan; }
  void SetSpan(TextSpan span) { mSpan = span; }

private:
  TextSpan mSpan;
};

class BoundExpressionNode : public BoundNode {
public:
  BoundExpressionNode() = default;
  virtual Value::Type Type() = 0;
  virtual Value::Type ElementType() const { return Value::Type::Unknown; }
  virtual ~BoundExpressionNode() {}
};

class BoundStatementNode : public BoundNode {};
