// Copyright (c) 2022 F. Lotfi All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "Binder.h"
#include "BoundAssignmentExpressionNode.h"
#include "BoundBinaryExpressionNode.h"
#include "BoundBlockStatementNode.h"
#include "BoundCallExpressionNode.h"
#include "BoundExpressionStatementNode.h"
#include "BoundForStatementNode.h"
#include "BoundFunctionDeclarationNode.h"
#include "BoundIdentifierExpressionNode.h"
#include "BoundIfStatementNode.h"
#include "BoundLiteralExpressionNode.h"
#include "BoundUnaryExpressionNode.h"
#include "BoundVariableDeclarationNode.h"
#include "BoundWhileStatementNode.h"
#include "Syntax/AssignmentExpressionNode.h"
#include "Syntax/BinaryExpressionNode.h"
#include "Syntax/BlockStatementSyntaxNode.h"
#include "Syntax/CallExpressionNode.h"
#include "Syntax/ExpressionStatementSyntaxNode.h"
#include "Syntax/ForStatementSyntaxNode.h"
#include "Syntax/FunctionDeclarationSyntaxNode.h"
#include "Syntax/IdentifierExpressionNode.h"
#include "Syntax/IfStatementSyntaxNode.h"
#include "Syntax/LiteralExpressionNode.h"
#include "Syntax/ParenthesizedExpressionNode.h"
#include "Syntax/StatementSyntaxNode.h"
#include "Syntax/UnaryExpressionNode.h"
#include "Syntax/VariableDeclarationSyntaxNode.h"
#include "Syntax/WhileStatementSyntaxNode.h"

#include "Symbol/SymbolTableMgr.h"
#include "Symbol/VariableSymbol.h"

#include <sstream>

Binder::Binder() : mRecords("Binder"), mScope(nullptr) {
  SymbolTableMgr::init();
  mScope = SymbolTableMgr::getGlobalScope();
}

std::unique_ptr<BoundStatementNode>
Binder::BindCompilationUnit(CompilationUnitSyntaxNode *syntax) {
  auto statements = std::vector<std::unique_ptr<BoundStatementNode>>();
  for (const auto &statement : syntax->Statements()) {
    statements.push_back(BindStatement(statement.get()));
  }
  return std::make_unique<BoundBlockStatementNode>(std::move(statements));
}

std::unique_ptr<BoundStatementNode>
Binder::BindStatement(StatementSyntaxNode *syntax) {
  switch (syntax->Kind().GetValue()) {
  case SyntaxKind::BlockStatement:
    return BindBlockStatement(dynamic_cast<BlockStatementSyntaxNode *>(syntax));
  case SyntaxKind::VariableDeclaration:
    return BindVariableDeclaration(
        dynamic_cast<VariableDeclarationSyntaxNode *>(syntax));
  case SyntaxKind::FunctionDeclaration:
    return BindFunctionDeclaration(
        dynamic_cast<FunctionDeclarationSyntaxNode *>(syntax));
  case SyntaxKind::ExpressionStatement:
    return BindExpressionStatement(
        dynamic_cast<ExpressionStatementSyntaxNode *>(syntax));
  case SyntaxKind::IfStatement:
    return BindIfStatement(dynamic_cast<IfStatementSyntaxNode *>(syntax));
  case SyntaxKind::WhileStatement:
    return BindWhileStatement(dynamic_cast<WhileStatementSyntaxNode *>(syntax));
  case SyntaxKind::ForStatement:
    return BindForStatement(dynamic_cast<ForStatementSyntaxNode *>(syntax));
  default:
    std::stringstream diagmsg;
    diagmsg << "Unexpected syntax " << SyntaxTokenToStrMap.at(syntax->Kind());
    throw std::runtime_error(diagmsg.str());
  }
}

std::unique_ptr<BoundStatementNode>
Binder::BindIfStatement(IfStatementSyntaxNode *syntax) {
  auto condition = BindExpression(syntax->Condition().get());
  if (condition->Type() != Value::Type::Boolean) {
    throw std::runtime_error("If condition must be Boolean");
  }
  auto thenStatement =
      BindStatement(const_cast<StatementSyntaxNode *>(syntax->ThenStatement()));
  std::unique_ptr<BoundStatementNode> elseStatement;
  if (syntax->ElseStatement()) {
    elseStatement = BindStatement(
        const_cast<StatementSyntaxNode *>(syntax->ElseStatement()));
  }
  return std::make_unique<BoundIfStatementNode>(
      std::move(condition), std::move(thenStatement), std::move(elseStatement));
}

std::unique_ptr<BoundStatementNode>
Binder::BindWhileStatement(WhileStatementSyntaxNode *syntax) {
  auto condition = BindExpression(syntax->Condition().get());
  if (condition->Type() != Value::Type::Boolean) {
    throw std::runtime_error("While condition must be Boolean");
  }
  auto body = BindStatement(const_cast<StatementSyntaxNode *>(syntax->Body()));
  return std::make_unique<BoundWhileStatementNode>(std::move(condition),
                                                   std::move(body));
}

std::unique_ptr<BoundStatementNode>
Binder::BindForStatement(ForStatementSyntaxNode *syntax) {
  auto parentScope = mScope;
  mScope = std::make_shared<Scope>(Scope::ScopeKind::Global, nullptr, "",
                                   parentScope);
  std::unique_ptr<BoundStatementNode> initializer;
  if (syntax->Initializer()) {
    initializer =
        BindStatement(const_cast<StatementSyntaxNode *>(syntax->Initializer()));
  }
  auto condition = BindExpression(syntax->Condition().get());
  if (condition->Type() != Value::Type::Boolean) {
    throw std::runtime_error("For condition must be Boolean");
  }
  auto increment = BindExpression(syntax->Increment().get());
  auto body = BindStatement(const_cast<StatementSyntaxNode *>(syntax->Body()));
  mScope = parentScope;
  return std::make_unique<BoundForStatementNode>(
      std::move(initializer), std::move(condition), std::move(increment),
      std::move(body));
}

std::unique_ptr<BoundStatementNode>
Binder::BindBlockStatement(BlockStatementSyntaxNode *syntax) {
  auto statements = std::vector<std::unique_ptr<BoundStatementNode>>();
  auto parentScope = mScope;
  mScope = std::make_shared<Scope>(Scope::ScopeKind::Global, nullptr, "",
                                   parentScope);

  for (const auto &statementSyntaxNode : syntax->Statements()) {
    auto boundStatement = BindStatement(statementSyntaxNode.get());
    statements.push_back(std::move(boundStatement));
  }

  mScope = parentScope;

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

  mScope->insert(variable);

  return std::make_unique<BoundVariableDeclarationNode>(variable,
                                                        std::move(initializer));
}

std::unique_ptr<BoundStatementNode>
Binder::BindFunctionDeclaration(FunctionDeclarationSyntaxNode *syntax) {
  auto parameters = std::vector<std::shared_ptr<VariableSymbol>>();
  for (const auto &parameter : syntax->Parameters()) {
    parameters.push_back(std::make_shared<VariableSymbol>(
        parameter->Text(), true, Value::Type::Unknown));
  }

  auto function = std::make_shared<FunctionSymbol>(syntax->Identifier()->Text(),
                                                   parameters);
  mScope->insert(function);

  auto parentScope = mScope;
  mScope = std::make_shared<Scope>(Scope::ScopeKind::Global, nullptr, "",
                                   parentScope);
  for (const auto &parameter : parameters) {
    mScope->insert(parameter);
  }
  auto body = BindBlockStatement(
      const_cast<BlockStatementSyntaxNode *>(syntax->Body()));
  mScope = parentScope;

  return std::make_unique<BoundFunctionDeclarationNode>(function,
                                                        std::move(body));
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
  if (CallExpressionNode *callExpression =
          dynamic_cast<CallExpressionNode *>(node)) {
    return std::move(BindCallExpression(callExpression));
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
    auto localVariable = mScope->lookupLocal(name);
    if (localVariable != VariableSymbol::failSymbol()) {
      localVariable = std::make_shared<VariableSymbol>(name, isReadOnly,
                                                       boundExpression->Type());
      mScope->insert(localVariable);
    } else {
      auto variable = mScope->lookup(name);
      if (variable == VariableSymbol::failSymbol()) {
        variable = std::make_shared<VariableSymbol>(name, isReadOnly,
                                                    boundExpression->Type());
        mScope->insert(variable);
      }
      localVariable = variable;
    }
    return std::make_unique<BoundAssignmentExpressionNode>(
        localVariable, std::move(boundExpression), boundOperator);
  } else {

    std::shared_ptr<VariableSymbol> existingVariable = mScope->lookup(name);
    if (existingVariable == VariableSymbol::failSymbol()) {
      mRecords.ReportUndefinedIdentifier(assignment->IdentifierToken());
      throw std::runtime_error("Failed to Bind LiteralExpression");
    }
    return std::make_unique<BoundAssignmentExpressionNode>(
        existingVariable, std::move(boundExpression), boundOperator);
  }
}

std::unique_ptr<BoundExpressionNode>
Binder::BindCallExpression(CallExpressionNode *call) {
  auto boundArguments = std::vector<std::unique_ptr<BoundExpressionNode>>();
  for (const auto &argument : call->Arguments()) {
    boundArguments.push_back(BindExpression(argument.get()));
  }

  auto name = call->IdentifierToken()->Text();
  auto function = mScope->lookupFunction(name);
  if (function == FunctionSymbol::failSymbol()) {
    mRecords.ReportUndefinedFunction(call->IdentifierToken());
    return std::make_unique<BoundLiteralExpressionNode>(0);
  }
  if (boundArguments.size() != function->Parameters().size()) {
    mRecords.ReportWrongArgumentCount(call->IdentifierToken(), name,
                                      function->Parameters().size(),
                                      boundArguments.size());
    return std::make_unique<BoundLiteralExpressionNode>(0);
  }

  return std::make_unique<BoundCallExpressionNode>(
      name, std::move(boundArguments), Value::Type::Unknown);
}

std::unique_ptr<BoundExpressionNode>
Binder::BindIdentifierExpression(IdentifierExpressionNode *identifier) {
  std::string name = identifier->IdentifierToken()->Text();
  std::shared_ptr<VariableSymbol> variable = mScope->lookup(name);
  if (variable == VariableSymbol::failSymbol()) {
    mRecords.ReportUndefinedIdentifier(identifier->IdentifierToken());
    throw std::runtime_error("Failed to Bind LiteralExpression");
  }

  return std::make_unique<BoundIdentifierExpressionNode>(variable);
}
