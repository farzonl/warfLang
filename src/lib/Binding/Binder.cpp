// Copyright (c) 2022 F. Lotfi All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "Binder.h"
#include "BoundAssignmentExpressionNode.h"
#include "BoundBinaryExpressionNode.h"
#include "BoundBlockStatement.h"
#include "BoundExpressionStatement.h"
#include "BoundIdentifierExpressionNode.h"
#include "BoundLiteralExpressionNode.h"
#include "BoundUnaryExpressionNode.h"
#include "Syntax/AssignmentExpressionNode.h"
#include "Syntax/BinaryExpressionNode.h"
#include "Syntax/IdentifierExpressionNode.h"
#include "Syntax/LiteralExpressionNode.h"
#include "Syntax/ParenthesizedExpressionNode.h"
#include "Syntax/UnaryExpressionNode.h"

#include "Symbol/SymbolTableMgr.h"
#include "Symbol/VariableSymbol.h"

#include <sstream>

class StatementSyntax {
public:
  virtual ~StatementSyntax() {}
};

class ExpressionStatementSyntax : public StatementSyntax {
public:
  ExpressionNode *Expression() { return nullptr; }
  ~ExpressionStatementSyntax() {}
};

class BlockStatementSyntax : public StatementSyntax {
public:
  std::vector<std::shared_ptr<StatementSyntax>> Statements;
  ~BlockStatementSyntax() {}
};

std::unique_ptr<BoundStatement>
Binder::BindStatement(std::shared_ptr<StatementSyntax> syntax) {
  if (BlockStatementSyntax *blockStatement =
          dynamic_cast<BlockStatementSyntax *>(syntax.get())) {
    return std::move(BindBlockStatement(blockStatement));
  }
  if (ExpressionStatementSyntax *expressionStatement =
          dynamic_cast<ExpressionStatementSyntax *>(syntax.get())) {
    return std::move(BindExpressionStatement(expressionStatement));
  }
  std::stringstream diagmsg;
  // diagmsg << "Unexpected syntax " << SyntaxTypeStrMap.at(syntax->GetValue());
  throw std::runtime_error(diagmsg.str());
}

std::unique_ptr<BoundStatement>
Binder::BindBlockStatement(BlockStatementSyntax *syntax) {
  std::vector<std::unique_ptr<BoundStatement>> statements;
  //_scope = new BoundScope(_scope);
  for (auto statementSyntax : syntax->Statements) {
    auto statement = BindStatement(statementSyntax);
    statements.push_back(std::move(statement));
  }

  //_scope = _scope.Parent;
  return std::make_unique<BoundBlockStatement>(statements);
}

std::unique_ptr<BoundStatement>
Binder::BindExpressionStatement(ExpressionStatementSyntax *syntax) {
  auto expression = BindExpression(syntax->Expression());
  return std::make_unique<BoundExpressionStatement>(std::move(expression));
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
  diagmsg << "Unexpected syntax " << SyntaxTokenToStrMap.at(node->Type());
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
      BoundUnaryOperator::Bind(unary->Operator()->Type(),
                               boundOperand->GetType());
  if (boundOperator == BoundUnaryOperator::GetBindFailure()) {
    mRecords.ReportUndefinedUnaryOperator(unary->Operator(),
                                          boundOperand->GetType());
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
      BoundBinaryOperator::Bind(binary->Operator()->Type(),
                                boundLeft->GetType(), boundRight->GetType());
  if (boundOperator == BoundBinaryOperator::GetBindFailure()) {
    mRecords.ReportUndefinedBinaryOperator(
        binary->Operator(), boundLeft->GetType(), boundRight->GetType());
    return boundLeft;
  }
  return std::make_unique<BoundBinaryExpressionNode>(
      std::move(boundLeft), boundOperator, std::move(boundRight));
}

std::unique_ptr<BoundExpressionNode>
Binder::BindAssignmentExpression(AssignmentExpressionNode *assignment) {
  std::string name = assignment->IdentifierToken()->Text();
  auto boundExpression = BindExpression(assignment->Expression());
  const std::shared_ptr<BoundAssignmentOperator> boundOperator =
      BoundAssignmentOperator::Bind(assignment->AssignmentToken()->Type(),
                                    boundExpression->GetType());
  if (assignment->AssignmentToken()->Type() == SyntaxType::EqualsToken) {
    auto newVar =
        std::make_shared<VariableSymbol>(name, boundExpression->GetType());
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
      return std::make_unique<BoundLiteralExpressionNode>(0);
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
    return std::make_unique<BoundLiteralExpressionNode>(0);
  }

  return std::make_unique<BoundIdentifierExpressionNode>(variable);
}
