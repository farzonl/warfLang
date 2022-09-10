/*
 * Copyright (c) 2022 F. Lotfi All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#pragma once

#include <memory>

#include "BoundExpressionNode.h"
#include "Error/Record.h"
#include "Syntax/ExpressionNode.h"

class LiteralExpressionNode;
class UnaryExpressionNode;
class BinaryExpressionNode;
class AssignmentExpressionNode;
class IdentifierExpressionNode;

class Binder {
public:
  std::unique_ptr<BoundExpressionNode> BindExpression(ExpressionNode *syntax);
  const Records &Errors() const { return mRecords; }
  Binder() : mRecords("Binder") {}

private:
  Records mRecords;
  std::unique_ptr<BoundExpressionNode>
  BindLiteralExpression(LiteralExpressionNode *literal);
  std::unique_ptr<BoundExpressionNode>
  BindUnaryExpression(UnaryExpressionNode *unary);
  std::unique_ptr<BoundExpressionNode>
  BindBinaryExpression(BinaryExpressionNode *binary);
  std::unique_ptr<BoundExpressionNode>
  BindAssignmentExpression(AssignmentExpressionNode *assignment);
  std::unique_ptr<BoundExpressionNode>
  BindIdentifierExpression(IdentifierExpressionNode *identifier);
};