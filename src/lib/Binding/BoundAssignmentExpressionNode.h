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
  ModuloAndAssign,
  BitwiseAndAndAssign,
  BitwiseOrAndAssign,
  BitwiseXorAndAssign,
};

class BoundAssignmentOperator {
public:
  static const std::shared_ptr<BoundAssignmentOperator>
  Bind(SyntaxKind syntaxKind, Value::Type lhsOperandType,
       Value::Type rhsOperandType);
  static const std::shared_ptr<BoundAssignmentOperator> GetBindFailure();
  SyntaxKind GetSyntaxKind();
  Value::Type LeftHandExpressionType();
  Value::Type RightHandExpressionType();
  Value::Type LhsType() const { return mLhsType; }
  Value::Type RhsType() const { return mRhsType; }
  BoundAssignmentOperatorKind AssignmentKind();
  BoundAssignmentOperator(SyntaxKind syntaxKind, Value::Type lhsOperandType,
                          Value::Type rhsOperandType);

private:
  SyntaxKind mSyntaxKind;
  Value::Type mLhsType;
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
  virtual BoundNodeKind Kind() override;
  BoundExpressionNode *BoundExpression();
  std::string Identifier();
  virtual Value::Type Type() override;
  virtual Value::Type ElementType() const override {
    return mBoundExpression->ElementType();
  }
  std::shared_ptr<VariableSymbol> Variable();
  BoundAssignmentOperatorKind OperatorKind();

private:
  std::unique_ptr<BoundExpressionNode> mBoundExpression;
  std::shared_ptr<VariableSymbol> mVariable;
  std::shared_ptr<BoundAssignmentOperator> mAssignmentOperator;
};
