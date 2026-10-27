// Copyright (c) 2022 F. Lotfi All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "Binder.h"
#include "BoundArrayLiteralExpressionNode.h"
#include "BoundAssignmentExpressionNode.h"
#include "BoundBinaryExpressionNode.h"
#include "BoundBlockStatementNode.h"
#include "BoundBreakStatementNode.h"
#include "BoundCallExpressionNode.h"
#include "BoundContinueStatementNode.h"
#include "BoundDoWhileStatementNode.h"
#include "BoundExpressionStatementNode.h"
#include "BoundForStatementNode.h"
#include "BoundFunctionDeclarationNode.h"
#include "BoundIdentifierExpressionNode.h"
#include "BoundIfStatementNode.h"
#include "BoundIndexAssignmentExpressionNode.h"
#include "BoundIndexExpressionNode.h"
#include "BoundLiteralExpressionNode.h"
#include "BoundReturnStatementNode.h"
#include "BoundUnaryExpressionNode.h"
#include "BoundVariableDeclarationNode.h"
#include "BoundWhileStatementNode.h"
#include "Syntax/ArrayLiteralExpressionNode.h"
#include "Syntax/AssignmentExpressionNode.h"
#include "Syntax/BinaryExpressionNode.h"
#include "Syntax/BlockStatementSyntaxNode.h"
#include "Syntax/BreakStatementSyntaxNode.h"
#include "Syntax/CallExpressionNode.h"
#include "Syntax/ContinueStatementSyntaxNode.h"
#include "Syntax/DoWhileStatementSyntaxNode.h"
#include "Syntax/ExpressionStatementSyntaxNode.h"
#include "Syntax/ForStatementSyntaxNode.h"
#include "Syntax/FunctionDeclarationSyntaxNode.h"
#include "Syntax/IdentifierExpressionNode.h"
#include "Syntax/IfStatementSyntaxNode.h"
#include "Syntax/IndexAssignmentExpressionNode.h"
#include "Syntax/IndexExpressionNode.h"
#include "Syntax/LiteralExpressionNode.h"
#include "Syntax/ParenthesizedExpressionNode.h"
#include "Syntax/ReturnStatementSyntaxNode.h"
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
  auto block = std::make_unique<BoundBlockStatementNode>(std::move(statements));
  if (syntax) {
    block->SetSpan(syntax->Span());
  }
  return block;
}

std::unique_ptr<BoundStatementNode>
Binder::BindStatement(StatementSyntaxNode *syntax) {
  std::unique_ptr<BoundStatementNode> bound;
  switch (syntax->Kind().GetValue()) {
  case SyntaxKind::BlockStatement:
    bound =
        BindBlockStatement(dynamic_cast<BlockStatementSyntaxNode *>(syntax));
    break;
  case SyntaxKind::VariableDeclaration:
    bound = BindVariableDeclaration(
        dynamic_cast<VariableDeclarationSyntaxNode *>(syntax));
    break;
  case SyntaxKind::FunctionDeclaration:
    bound = BindFunctionDeclaration(
        dynamic_cast<FunctionDeclarationSyntaxNode *>(syntax));
    break;
  case SyntaxKind::ExpressionStatement:
    bound = BindExpressionStatement(
        dynamic_cast<ExpressionStatementSyntaxNode *>(syntax));
    break;
  case SyntaxKind::IfStatement:
    bound = BindIfStatement(dynamic_cast<IfStatementSyntaxNode *>(syntax));
    break;
  case SyntaxKind::WhileStatement:
    bound =
        BindWhileStatement(dynamic_cast<WhileStatementSyntaxNode *>(syntax));
    break;
  case SyntaxKind::DoWhileStatement:
    bound = BindDoWhileStatement(
        dynamic_cast<DoWhileStatementSyntaxNode *>(syntax));
    break;
  case SyntaxKind::ForStatement:
    bound = BindForStatement(dynamic_cast<ForStatementSyntaxNode *>(syntax));
    break;
  case SyntaxKind::BreakStatement:
    bound =
        BindBreakStatement(dynamic_cast<BreakStatementSyntaxNode *>(syntax));
    break;
  case SyntaxKind::ContinueStatement:
    bound = BindContinueStatement(
        dynamic_cast<ContinueStatementSyntaxNode *>(syntax));
    break;
  case SyntaxKind::ReturnStatement:
    bound =
        BindReturnStatement(dynamic_cast<ReturnStatementSyntaxNode *>(syntax));
    break;
  default:
    std::stringstream diagmsg;
    diagmsg << "Unexpected syntax " << SyntaxTokenToStrMap.at(syntax->Kind());
    throw std::runtime_error(diagmsg.str());
  }
  if (bound && syntax) {
    bound->SetSpan(syntax->Span());
  }
  return bound;
}

Value::Type Binder::InferStatementType(const BoundStatementNode *node) {
  if (!node) {
    return Value::Type::Unknown;
  }
  if (auto *ret = dynamic_cast<const BoundReturnStatementNode *>(node)) {
    return ret->Expression()
               ? const_cast<BoundExpressionNode *>(ret->Expression())->Type()
               : Value::Type::Unknown;
  }
  if (auto *exprStmt =
          dynamic_cast<const BoundExpressionStatementNode *>(node)) {
    return const_cast<BoundExpressionNode *>(exprStmt->Expression())->Type();
  }
  if (auto *varDecl =
          dynamic_cast<const BoundVariableDeclarationNode *>(node)) {
    return const_cast<BoundExpressionNode *>(varDecl->Initializer())->Type();
  }
  if (auto *ifStmt = dynamic_cast<const BoundIfStatementNode *>(node)) {
    auto thenType = InferStatementType(ifStmt->ThenStatement());
    if (thenType != Value::Type::Unknown) {
      return thenType;
    }
    return InferStatementType(ifStmt->ElseStatement());
  }
  if (auto *block = dynamic_cast<const BoundBlockStatementNode *>(node)) {
    const BoundStatementNode *lastStmt = nullptr;
    for (const auto &stmt : block->Statements()) {
      lastStmt = stmt.get();
      if (auto *ret =
              dynamic_cast<const BoundReturnStatementNode *>(stmt.get())) {
        if (ret->Expression()) {
          auto exprType =
              const_cast<BoundExpressionNode *>(ret->Expression())->Type();
          if (exprType != Value::Type::Unknown) {
            return exprType;
          }
        }
      }
      if (auto *ifStmt =
              dynamic_cast<const BoundIfStatementNode *>(stmt.get())) {
        auto ifType = InferStatementType(ifStmt);
        if (ifType != Value::Type::Unknown) {
          return ifType;
        }
      }
    }
    if (lastStmt) {
      return InferStatementType(lastStmt);
    }
  }
  return Value::Type::Unknown;
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
  mLoopDepth++;
  auto body = BindStatement(const_cast<StatementSyntaxNode *>(syntax->Body()));
  mLoopDepth--;
  return std::make_unique<BoundWhileStatementNode>(std::move(condition),
                                                   std::move(body));
}

std::unique_ptr<BoundStatementNode>
Binder::BindDoWhileStatement(DoWhileStatementSyntaxNode *syntax) {
  mLoopDepth++;
  auto body = BindStatement(const_cast<StatementSyntaxNode *>(syntax->Body()));
  mLoopDepth--;
  auto condition = BindExpression(syntax->Condition().get());
  if (condition->Type() != Value::Type::Boolean) {
    throw std::runtime_error("DoWhile condition must be Boolean");
  }
  return std::make_unique<BoundDoWhileStatementNode>(std::move(body),
                                                     std::move(condition));
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
  mLoopDepth++;
  auto body = BindStatement(const_cast<StatementSyntaxNode *>(syntax->Body()));
  mLoopDepth--;
  mScope = parentScope;
  return std::make_unique<BoundForStatementNode>(
      std::move(initializer), std::move(condition), std::move(increment),
      std::move(body));
}

std::unique_ptr<BoundStatementNode>
Binder::BindBreakStatement(BreakStatementSyntaxNode *syntax) {
  if (mLoopDepth == 0) {
    mRecords.ReportInvalidBreakOrContinue(syntax->BreakKeyword());
  }
  return std::make_unique<BoundBreakStatementNode>();
}

std::unique_ptr<BoundStatementNode>
Binder::BindContinueStatement(ContinueStatementSyntaxNode *syntax) {
  if (mLoopDepth == 0) {
    mRecords.ReportInvalidBreakOrContinue(syntax->ContinueKeyword());
  }
  return std::make_unique<BoundContinueStatementNode>();
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

  if (syntax->TypeAnnotation() != Value::Type::Unknown &&
      initializer->Type() != Value::Type::Unknown &&
      syntax->TypeAnnotation() != initializer->Type()) {
    mRecords.ReportWrongVariableType(syntax->Identifier(), name,
                                     syntax->TypeAnnotation(),
                                     initializer->Type());
  }

  auto varType = syntax->TypeAnnotation() != Value::Type::Unknown
                     ? syntax->TypeAnnotation()
                     : initializer->Type();
  auto variable = std::make_shared<VariableSymbol>(name, isReadOnly, varType,
                                                   initializer->ElementType());

  mScope->insert(variable);

  return std::make_unique<BoundVariableDeclarationNode>(variable,
                                                        std::move(initializer));
}

std::unique_ptr<BoundStatementNode>
Binder::BindFunctionDeclaration(FunctionDeclarationSyntaxNode *syntax) {
  auto parameters = std::vector<std::shared_ptr<VariableSymbol>>();
  const auto &paramTypes = syntax->ParameterTypes();
  for (size_t i = 0; i < syntax->Parameters().size(); i++) {
    Value::Type paramType =
        i < paramTypes.size() ? paramTypes[i] : Value::Type::Unknown;
    parameters.push_back(std::make_shared<VariableSymbol>(
        syntax->Parameters()[i]->Text(), false, paramType));
  }

  auto function =
      std::make_shared<FunctionSymbol>(syntax->Identifier()->Text(), parameters,
                                       syntax->Body(), syntax->ReturnType());
  mScope->insert(function);

  return std::make_unique<BoundFunctionDeclarationNode>(function);
}

std::unique_ptr<BoundStatementNode>
Binder::BindReturnStatement(ReturnStatementSyntaxNode *syntax) {
  std::unique_ptr<BoundExpressionNode> expr;
  if (syntax->Expression()) {
    expr = BindExpression(const_cast<ExpressionNode *>(syntax->Expression()));
  }
  if (!mActiveFunctionStack.empty()) {
    const auto &fnName = mActiveFunctionStack.back();
    auto expectedType = mActiveFunctionReturnTypes[fnName];
    auto actualType = expr ? expr->Type() : Value::Type::Unknown;
    if (expectedType == Value::Type::Unknown) {
      if (actualType != Value::Type::Unknown) {
        mActiveFunctionReturnTypes[fnName] = actualType;
      }
    } else {
      if (actualType != Value::Type::Unknown && actualType != expectedType) {
        mRecords.ReportWrongFunctionReturnType(syntax->ReturnKeyword(), fnName,
                                               expectedType, actualType);
      }
    }
  }
  return std::make_unique<BoundReturnStatementNode>(std::move(expr));
}

std::unique_ptr<BoundStatementNode>
Binder::BindExpressionStatement(ExpressionStatementSyntaxNode *syntax) {
  auto expression = BindExpression(syntax->Expression());
  return std::make_unique<BoundExpressionStatementNode>(std::move(expression));
}

std::unique_ptr<BoundExpressionNode>
Binder::BindExpression(ExpressionNode *node) {
  std::unique_ptr<BoundExpressionNode> bound;
  if (LiteralExpressionNode *literal =
          dynamic_cast<LiteralExpressionNode *>(node)) {
    bound = BindLiteralExpression(literal);
  } else if (UnaryExpressionNode *unaryExpression =
                 dynamic_cast<UnaryExpressionNode *>(node)) {
    bound = BindUnaryExpression(unaryExpression);
  } else if (BinaryExpressionNode *binaryExpression =
                 dynamic_cast<BinaryExpressionNode *>(node)) {
    bound = BindBinaryExpression(binaryExpression);
  } else if (ParenthesizedExpressionNode *parenthesizedExpression =
                 dynamic_cast<ParenthesizedExpressionNode *>(node)) {
    bound = BindExpression(parenthesizedExpression->Expression());
  } else if (ArrayLiteralExpressionNode *arrayLiteralExpression =
                 dynamic_cast<ArrayLiteralExpressionNode *>(node)) {
    bound = BindArrayLiteralExpression(arrayLiteralExpression);
  } else if (IndexExpressionNode *indexExpression =
                 dynamic_cast<IndexExpressionNode *>(node)) {
    bound = BindIndexExpression(indexExpression);
  } else if (IndexAssignmentExpressionNode *indexAssignmentExpression =
                 dynamic_cast<IndexAssignmentExpressionNode *>(node)) {
    bound = BindIndexAssignmentExpression(indexAssignmentExpression);
  } else if (AssignmentExpressionNode *assignmentExpression =
                 dynamic_cast<AssignmentExpressionNode *>(node)) {
    bound = BindAssignmentExpression(assignmentExpression);
  } else if (CallExpressionNode *callExpression =
                 dynamic_cast<CallExpressionNode *>(node)) {
    bound = BindCallExpression(callExpression);
  } else if (IdentifierExpressionNode *identifierExpression =
                 dynamic_cast<IdentifierExpressionNode *>(node)) {
    bound = BindIdentifierExpression(identifierExpression);
  } else {
    std::stringstream diagmsg;
    diagmsg << "Unexpected syntax " << SyntaxTokenToStrMap.at(node->Kind());
    throw std::runtime_error(diagmsg.str());
  }
  if (bound && node) {
    bound->SetSpan(node->Span());
  }
  return bound;
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
Binder::BindArrayLiteralExpression(ArrayLiteralExpressionNode *arrayLiteral) {
  auto boundElements = std::vector<std::unique_ptr<BoundExpressionNode>>();
  for (const auto &element : arrayLiteral->Elements()) {
    boundElements.push_back(BindExpression(element.get()));
  }
  Value::Type elementType = Value::Type::Number;
  if (!boundElements.empty()) {
    elementType = boundElements[0]->Type();
    for (size_t i = 1; i < boundElements.size(); i++) {
      if (boundElements[i]->Type() != elementType) {
        mRecords.ReportHeterogeneousArray(arrayLiteral->OpenBracket()->Span(),
                                          elementType,
                                          boundElements[i]->Type());
      }
    }
  }
  return std::make_unique<BoundArrayLiteralExpressionNode>(
      std::move(boundElements), elementType);
}

std::unique_ptr<BoundExpressionNode>
Binder::BindIndexExpression(IndexExpressionNode *indexExpression) {
  auto boundTarget = BindExpression(indexExpression->Target());
  auto boundIndex = BindExpression(indexExpression->Index());
  if (boundTarget->Type() != Value::Type::Array) {
    mRecords.ReportCannotIndexType(indexExpression->OpenBracket()->Span(),
                                   boundTarget->Type());
  }
  if (boundIndex->Type() != Value::Type::Number) {
    mRecords.ReportInvalidArrayIndexType(indexExpression->OpenBracket()->Span(),
                                         boundIndex->Type());
  }
  Value::Type elementType = boundTarget->ElementType() != Value::Type::Unknown
                                ? boundTarget->ElementType()
                                : Value::Type::Number;
  return std::make_unique<BoundIndexExpressionNode>(
      std::move(boundTarget), std::move(boundIndex), elementType);
}

std::unique_ptr<BoundExpressionNode> Binder::BindIndexAssignmentExpression(
    IndexAssignmentExpressionNode *indexAssignment) {
  auto *indexExpr = indexAssignment->IndexExpression();
  auto boundTarget = BindExpression(indexExpr->Target());
  auto boundIndex = BindExpression(indexExpr->Index());
  auto boundValue = BindExpression(indexAssignment->ValueExpression());
  if (boundTarget->Type() != Value::Type::Array) {
    mRecords.ReportCannotIndexType(indexExpr->OpenBracket()->Span(),
                                   boundTarget->Type());
  }
  if (boundIndex->Type() != Value::Type::Number) {
    mRecords.ReportInvalidArrayIndexType(indexExpr->OpenBracket()->Span(),
                                         boundIndex->Type());
  }
  if (boundTarget->Type() == Value::Type::Array &&
      boundTarget->ElementType() != Value::Type::Unknown &&
      boundValue->Type() != boundTarget->ElementType()) {
    mRecords.ReportArrayElementTypeMismatch(
        indexAssignment->EqualsToken()->Span(), boundTarget->ElementType(),
        boundValue->Type());
  }
  return std::make_unique<BoundIndexAssignmentExpressionNode>(
      std::move(boundTarget), std::move(boundIndex), std::move(boundValue));
}

std::unique_ptr<BoundExpressionNode>
Binder::BindAssignmentExpression(AssignmentExpressionNode *assignment) {
  std::string name = assignment->IdentifierToken()->Text();
  auto boundExpression = BindExpression(assignment->Expression());
  std::shared_ptr<VariableSymbol> existingVariable = mScope->lookup(name);
  if (existingVariable != VariableSymbol::failSymbol() &&
      existingVariable->IsReadOnly()) {
    mRecords.ReportCannotAssign(assignment->IdentifierToken());
    return boundExpression;
  }

  if (assignment->AssignmentToken()->Kind() == SyntaxKind::EqualsToken) {
    const std::shared_ptr<BoundAssignmentOperator> boundOperator =
        BoundAssignmentOperator::Bind(SyntaxKind::EqualsToken,
                                      boundExpression->Type(),
                                      boundExpression->Type());
    if (existingVariable == VariableSymbol::failSymbol() ||
        existingVariable->Type() != boundExpression->Type() ||
        existingVariable->ElementType() != boundExpression->ElementType()) {
      existingVariable = std::make_shared<VariableSymbol>(
          name, false, boundExpression->Type(), boundExpression->ElementType());
      mScope->insert(existingVariable);
    }
    return std::make_unique<BoundAssignmentExpressionNode>(
        existingVariable, std::move(boundExpression), boundOperator);
  } else {
    if (existingVariable == VariableSymbol::failSymbol()) {
      mRecords.ReportUndefinedIdentifier(assignment->IdentifierToken());
      throw std::runtime_error("Failed to Bind LiteralExpression");
    }
    const std::shared_ptr<BoundAssignmentOperator> boundOperator =
        BoundAssignmentOperator::Bind(assignment->AssignmentToken()->Kind(),
                                      existingVariable->Type(),
                                      boundExpression->Type());
    if (boundOperator == BoundAssignmentOperator::GetBindFailure()) {
      mRecords.ReportUndefinedBinaryOperator(assignment->AssignmentToken(),
                                             existingVariable->Type(),
                                             boundExpression->Type());
      return boundExpression;
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
    if (name == "print") {
      if (boundArguments.size() != 1) {
        mRecords.ReportWrongArgumentCount(call->IdentifierToken(), name, 1,
                                          boundArguments.size());
        return std::make_unique<BoundLiteralExpressionNode>(0);
      }
      auto argType = boundArguments[0]->Type();
      return std::make_unique<BoundCallExpressionNode>(
          name, std::move(boundArguments),
          std::vector<std::shared_ptr<VariableSymbol>>{}, nullptr, argType);
    }
    if (name == "len") {
      if (boundArguments.size() != 1) {
        mRecords.ReportWrongArgumentCount(call->IdentifierToken(), name, 1,
                                          boundArguments.size());
        return std::make_unique<BoundLiteralExpressionNode>(0);
      }
      if (boundArguments[0]->Type() != Value::Type::Array) {
        mRecords.ReportWrongArgumentType(call->IdentifierToken(), "arr",
                                         Value::Type::Array,
                                         boundArguments[0]->Type());
        return std::make_unique<BoundLiteralExpressionNode>(0);
      }
      return std::make_unique<BoundCallExpressionNode>(
          name, std::move(boundArguments),
          std::vector<std::shared_ptr<VariableSymbol>>{}, nullptr,
          Value::Type::Number);
    }
    mRecords.ReportUndefinedFunction(call->IdentifierToken());
    return std::make_unique<BoundLiteralExpressionNode>(0);
  }
  if (boundArguments.size() != function->Parameters().size()) {
    mRecords.ReportWrongArgumentCount(call->IdentifierToken(), name,
                                      function->Parameters().size(),
                                      boundArguments.size());
    return std::make_unique<BoundLiteralExpressionNode>(0);
  }

  for (size_t i = 0; i < function->Parameters().size(); i++) {
    auto expectedType = function->Parameters()[i]->Type();
    if (expectedType != Value::Type::Unknown &&
        boundArguments[i]->Type() != expectedType) {
      mRecords.ReportWrongArgumentType(call->IdentifierToken(),
                                       function->Parameters()[i]->Name(),
                                       expectedType, boundArguments[i]->Type());
      return std::make_unique<BoundLiteralExpressionNode>(0);
    }
  }

  auto activeIt = mActiveFunctionReturnTypes.find(name);
  if (activeIt != mActiveFunctionReturnTypes.end()) {
    Value::Type returnType = activeIt->second;
    if (returnType == Value::Type::Unknown) {
      returnType = function->ReturnType();
    }
    if (returnType == Value::Type::Unknown && !boundArguments.empty()) {
      returnType = boundArguments[0]->Type();
    }
    return std::make_unique<BoundCallExpressionNode>(
        name, std::move(boundArguments),
        std::vector<std::shared_ptr<VariableSymbol>>{}, nullptr, returnType);
  }

  auto parameters = std::vector<std::shared_ptr<VariableSymbol>>();
  auto parentScope = mScope;
  mScope = std::make_shared<Scope>(Scope::ScopeKind::Global, nullptr, "",
                                   parentScope);
  for (size_t i = 0; i < function->Parameters().size(); i++) {
    auto expectedType = function->Parameters()[i]->Type();
    auto paramType = expectedType != Value::Type::Unknown
                         ? expectedType
                         : boundArguments[i]->Type();
    auto parameter = std::make_shared<VariableSymbol>(
        function->Parameters()[i]->Name(), false, paramType,
        boundArguments[i]->ElementType());
    parameters.push_back(parameter);
    mScope->insert(parameter);
  }

  mActiveFunctionReturnTypes[name] = function->ReturnType();
  mActiveFunctionStack.push_back(name);
  auto body = BindBlockStatement(
      const_cast<BlockStatementSyntaxNode *>(function->Body()));
  mScope = parentScope;

  Value::Type returnType = mActiveFunctionReturnTypes[name];
  if (returnType == Value::Type::Unknown) {
    returnType = InferStatementType(body.get());
  }
  mActiveFunctionStack.pop_back();
  mActiveFunctionReturnTypes.erase(name);

  return std::make_unique<BoundCallExpressionNode>(
      name, std::move(boundArguments), std::move(parameters), std::move(body),
      returnType);
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
