// Copyright (c) 2022 F. Lotfi All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "Binder.h"
#include "BoundAssignmentExpressionNode.h"
#include "BoundBinaryExpressionNode.h"
#include "BoundBlockStatementNode.h"
#include "BoundExpressionStatementNode.h"
#include "BoundIdentifierExpressionNode.h"
#include "BoundLiteralExpressionNode.h"
#include "BoundUnaryExpressionNode.h"
#include "BoundVariableDeclarationNode.h"
#include "Syntax/AssignmentExpressionNode.h"
#include "Syntax/BinaryExpressionNode.h"
#include "Syntax/BlockStatementSyntaxNode.h"
#include "Syntax/ExpressionStatementSyntaxNode.h"
#include "Syntax/IdentifierExpressionNode.h"
#include "Syntax/LiteralExpressionNode.h"
#include "Syntax/ParenthesizedExpressionNode.h"
#include "Syntax/StatementSyntaxNode.h"
#include "Syntax/UnaryExpressionNode.h"
#include "Syntax/VariableDeclarationSyntaxNode.h"

#include "Symbol/SymbolTableMgr.h"
#include "Symbol/VariableSymbol.h"

#include <sstream>

std::unique_ptr<BoundStatementNode>
Binder::BindStatement(StatementSyntaxNode *syntax) {
  switch (syntax->Kind().GetValue()) {
  case SyntaxKind::BlockStatement:
    return BindBlockStatement(dynamic_cast<BlockStatementSyntaxNode *>(syntax));
  case SyntaxKind::VariableDeclaration:
    return BindVariableDeclaration(
        dynamic_cast<VariableDeclarationSyntaxNode *>(syntax));
  case SyntaxKind::ExpressionStatement:
    return BindExpressionStatement(
        dynamic_cast<ExpressionStatementSyntaxNode *>(syntax));
  default:
    std::stringstream diagmsg;
    diagmsg << "Unexpected syntax " << SyntaxTokenToStrMap.at(syntax->Kind());
    throw std::runtime_error(diagmsg.str());
  }
}

std::unique_ptr<BoundStatementNode>
Binder::BindBlockStatement(BlockStatementSyntaxNode *syntax) {
  auto statements = std::vector<std::unique_ptr<BoundStatementNode>>();

  for (const auto &statementSyntaxNode : syntax->Statements()) {
    auto boundStatement = BindStatement(statementSyntaxNode.get());
    statements.push_back(std::move(boundStatement));
  }

  return std::make_unique<BoundBlockStatementNode>(std::move(statements));
}

std::unique_ptr<BoundStatementNode>
Binder::BindVariableDeclaration(VariableDeclarationSyntaxNode *syntax) {
  auto name = syntax->Identifier()->Text();
  bool isReadOnly = syntax->Keyword()->Kind() == SyntaxKind::LetKeyword;
  auto initializer =
      BindExpression(const_cast<ExpressionNode *>(syntax->Initializer()));

  auto variable =
      std::make_shared<VariableSymbol>(name, isReadOnly, initializer->Type());

  // TODO should we handle scoping here?

  return std::make_unique<BoundVariableDeclarationNode>(variable,
                                                        std::move(initializer));
}

std::unique_ptr<BoundStatementNode>
Binder::BindExpressionStatement(ExpressionStatementSyntaxNode *syntax) {
  auto expression = BindExpression(syntax->Expression());
  return std::make_unique<BoundExpressionStatementNode>(std::move(expression));
}

std::unique_ptr<BoundExpressionNode>
Binder::BindExpression(ExpressionNode *node) {

  if (LiteralExpressionNode *literal =
          dynamic_cast<LiteralExpressionNode *>(node)) {
    return std::move(BindLiteralExpression(literal));
  }
  if (UnaryExpressionNode *unaryExpression =
          dynamic_cast<UnaryExpressionNode *>(node)) {
    return std::move(BindUnaryExpression(unaryExpression));
  }
  if (BinaryExpressionNode *binaryExpression =
          dynamic_cast<BinaryExpressionNode *>(node)) {
    return std::move(BindBinaryExpression(binaryExpression));
  }
  if (ParenthesizedExpressionNode *parenthesizedExpression =
          dynamic_cast<ParenthesizedExpressionNode *>(node)) {
    return std::move(BindExpression(parenthesizedExpression->Expression()));
  }
  if (AssignmentExpressionNode *assignmentExpression =
          dynamic_cast<AssignmentExpressionNode *>(node)) {
    return std::move(BindAssignmentExpression(assignmentExpression));
  }
  if (IdentifierExpressionNode *identifierExpression =
          dynamic_cast<IdentifierExpressionNode *>(node)) {
    return std::move(BindIdentifierExpression(identifierExpression));
  }
  std::stringstream diagmsg;
  diagmsg << "Unexpected syntax " << SyntaxTokenToStrMap.at(node->Kind());
  throw std::runtime_error(diagmsg.str());
  return nullptr;
}

std::unique_ptr<BoundExpressionNode>
Binder::BindLiteralExpression(LiteralExpressionNode *literal) {
  return std::make_unique<BoundLiteralExpressionNode>(literal);
}

std::unique_ptr<BoundExpressionNode>
Binder::BindUnaryExpression(UnaryExpressionNode *unary) {
  auto boundOperand = BindExpression(unary->Operand());
  const std::shared_ptr<BoundUnaryOperator> boundOperator =
      BoundUnaryOperator::Bind(unary->Operator()->Kind(), boundOperand->Type());
  if (boundOperator == BoundUnaryOperator::GetBindFailure()) {
    mRecords.ReportUndefinedUnaryOperator(unary->Operator(),
                                          boundOperand->Type());
    return boundOperand;
  }
  return std::make_unique<BoundUnaryExpressionNode>(boundOperator,
                                                    std::move(boundOperand));
}

std::unique_ptr<BoundExpressionNode>
Binder::BindBinaryExpression(BinaryExpressionNode *binary) {
  auto boundLeft = BindExpression(binary->Left());
  auto boundRight = BindExpression(binary->Right());
  const std::shared_ptr<BoundBinaryOperator> boundOperator =
      BoundBinaryOperator::Bind(binary->Operator()->Kind(), boundLeft->Type(),
                                boundRight->Type());
  if (boundOperator == BoundBinaryOperator::GetBindFailure()) {
    mRecords.ReportUndefinedBinaryOperator(
        binary->Operator(), boundLeft->Type(), boundRight->Type());
    return boundLeft;
  }
  return std::make_unique<BoundBinaryExpressionNode>(
      std::move(boundLeft), boundOperator, std::move(boundRight));
}

std::unique_ptr<BoundExpressionNode>
Binder::BindAssignmentExpression(AssignmentExpressionNode *assignment) {
  std::string name = assignment->IdentifierToken()->Text();
  bool isReadOnly = false; // TODO remove this line temp to fix compile issue
  auto boundExpression = BindExpression(assignment->Expression());
  const std::shared_ptr<BoundAssignmentOperator> boundOperator =
      BoundAssignmentOperator::Bind(assignment->AssignmentToken()->Kind(),
                                    boundExpression->Type());
  if (assignment->AssignmentToken()->Kind() == SyntaxKind::EqualsToken) {
    auto newVar = std::make_shared<VariableSymbol>(name, isReadOnly,
                                                   boundExpression->Type());
    auto existingVariable = SymbolTableMgr::find(name);
    if (existingVariable != VariableSymbol::failSymbol()) {
      SymbolTableMgr::modify(newVar, existingVariable->GetScopeName());
    } else {
      SymbolTableMgr::insert(newVar);
    }
    return std::make_unique<BoundAssignmentExpressionNode>(
        newVar, std::move(boundExpression), boundOperator);
  } else {

    std::shared_ptr<VariableSymbol> existingVariable =
        SymbolTableMgr::find(name);
    if (existingVariable == VariableSymbol::failSymbol()) {
      mRecords.ReportUndefinedIdentifier(assignment->IdentifierToken());
      throw std::runtime_error("Failed to Bind LiteralExpression");
    }
    return std::make_unique<BoundAssignmentExpressionNode>(
        existingVariable, std::move(boundExpression), boundOperator);
  }
}

std::unique_ptr<BoundExpressionNode>
Binder::BindIdentifierExpression(IdentifierExpressionNode *identifier) {
  std::string name = identifier->IdentifierToken()->Text();
  std::shared_ptr<VariableSymbol> variable = SymbolTableMgr::find(name);
  if (variable == VariableSymbol::failSymbol()) {
    mRecords.ReportUndefinedIdentifier(identifier->IdentifierToken());
    throw std::runtime_error("Failed to Bind LiteralExpression");
  }

  return std::make_unique<BoundIdentifierExpressionNode>(variable);
}
