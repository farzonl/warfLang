/*
 * Copyright (c) 2022 F. Lotfi All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#pragma once

#include <memory>

#include "BoundExpressionNode.h"
#include "Symbol/VariableSymbol.h"

enum class BoundAssignmentOperatorType {
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
  Bind(SyntaxType syntaxType, Type OperandType);
  SyntaxType GetSyntaxType();
  Type RightHandExpressionType();
  BoundAssignmentOperatorType AssignmentType();
  BoundAssignmentOperator(SyntaxType syntaxType, Type rhsOperandType);

private:
  SyntaxType mSyntaxType;
  Type mRhsType;
  BoundAssignmentOperatorType mAssigmentType;
  static const std::shared_ptr<BoundAssignmentOperator> sOperators[];
  BoundAssignmentOperator() = delete;
  void setOperatorType();
  friend class BoundAssignmentExpressionNode;
};
class BoundAssignmentExpressionNode : public BoundExpressionNode {
public:
  BoundAssignmentExpressionNode(
      std::shared_ptr<VariableSymbol> variable,
      std::unique_ptr<BoundExpressionNode> boundExpression,
      std::shared_ptr<BoundAssignmentOperator> assignmentOperator);
  virtual ~BoundAssignmentExpressionNode() {}
  virtual BoundNodeType NodeType() override;
  BoundExpressionNode *BoundExpression();
  std::string Identifier();
  virtual Type GetType() override;
  std::shared_ptr<VariableSymbol> Variable();
  BoundAssignmentOperatorType OperatorType();

private:
  std::unique_ptr<BoundExpressionNode> mBoundExpression;
  std::shared_ptr<VariableSymbol> mVariable;
  std::shared_ptr<BoundAssignmentOperator> mAssignmentOperator;
};
