/*
 * Copyright (c) 2022 F. Lotfi All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#pragma once

#include <memory>

#include "BoundExpressionNode.h"
#include "Error/Record.h"
#include "Symbol/Scope.h"
#include "Syntax/CompilationUnitSyntaxNode.h"
#include "Syntax/ExpressionNode.h"

class LiteralExpressionNode;
class UnaryExpressionNode;
class BinaryExpressionNode;
class AssignmentExpressionNode;
class IdentifierExpressionNode;
class StatementSyntaxNode;
class BlockStatementSyntaxNode;
class VariableDeclarationSyntaxNode;
class ExpressionStatementSyntaxNode;
class IfStatementSyntaxNode;
class WhileStatementSyntaxNode;
class ForStatementSyntaxNode;

class Binder {
public:
  std::unique_ptr<BoundExpressionNode> BindExpression(ExpressionNode *syntax);
  std::unique_ptr<BoundStatementNode>
  BindCompilationUnit(CompilationUnitSyntaxNode *syntax);
  std::unique_ptr<BoundStatementNode>
  BindStatement(StatementSyntaxNode *syntax);
  const Records &Errors() const { return mRecords; }
  Binder();

private:
  Records mRecords;
  std::shared_ptr<Scope> mScope;
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
  std::unique_ptr<BoundStatementNode>
  BindBlockStatement(BlockStatementSyntaxNode *syntax);
  std::unique_ptr<BoundStatementNode>
  BindVariableDeclaration(VariableDeclarationSyntaxNode *syntax);
  std::unique_ptr<BoundStatementNode>
  BindExpressionStatement(ExpressionStatementSyntaxNode *syntax);
  std::unique_ptr<BoundStatementNode>
  BindIfStatement(IfStatementSyntaxNode *syntax);
  std::unique_ptr<BoundStatementNode>
  BindWhileStatement(WhileStatementSyntaxNode *syntax);
  std::unique_ptr<BoundStatementNode>
  BindForStatement(ForStatementSyntaxNode *syntax);
};
