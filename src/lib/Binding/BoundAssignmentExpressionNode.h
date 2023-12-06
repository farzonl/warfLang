/*
 * Copyright (c) 2022 F. Lotfi All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#pragma once

#include <memory>

#include "BoundExpressionNode.h"
#include "Symbol/VariableSymbol.h"

enum class BoundAssignmentOperatorKind {
  Assignment,
  AddAndAssign,
  SubtractAndAssign,
  MultiplyAndAssign,
  DivideAndAssign,
  BitwiseAndAndAssign,
  BitwiseOrAndAssign,
  BitwiseXorAndAssign,
};

class BoundAssignmentOperator {
public:
  static const std::shared_ptr<BoundAssignmentOperator>
  Bind(SyntaxKind syntaxKind, Value::Type OperandType);
  SyntaxKind GetSyntaxKind();
  Value::Type RightHandExpressionType();
  BoundAssignmentOperatorKind AssignmentKind();
  BoundAssignmentOperator(SyntaxKind syntaxKind, Value::Type rhsOperandType);

private:
  SyntaxKind mSyntaxKind;
  Value::Type mRhsType;
  BoundAssignmentOperatorKind mAssigmentKind;
  static const std::shared_ptr<BoundAssignmentOperator> sOperators[];
  BoundAssignmentOperator() = delete;
  void setOperatorKind();
  friend class BoundAssignmentExpressionNode;
};
class BoundAssignmentExpressionNode : public BoundExpressionNode {
public:
  BoundAssignmentExpressionNode(
      std::shared_ptr<VariableSymbol> variable,
      std::unique_ptr<BoundExpressionNode> boundExpression,
      std::shared_ptr<BoundAssignmentOperator> assignmentOperator);
  virtual ~BoundAssignmentExpressionNode() {}
  virtual BoundNodeKind NodeKind() override;
  BoundExpressionNode *BoundExpression();
  std::string Identifier();
  virtual Value::Type Type() override;
  std::shared_ptr<VariableSymbol> Variable();
  BoundAssignmentOperatorKind OperatorKind();

private:
  std::unique_ptr<BoundExpressionNode> mBoundExpression;
  std::shared_ptr<VariableSymbol> mVariable;
  std::shared_ptr<BoundAssignmentOperator> mAssignmentOperator;
};
