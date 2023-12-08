/*
 * Copyright (c) 2022 F. Lotfi All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#pragma once
#include <memory>

#include "BlockStatementSyntaxNode.h"
#include "CompilationUnitSyntaxNode.h"
#include "ExpressionNode.h"
#include "ExpressionStatementSyntaxNode.h"
#include "StatementSyntaxNode.h"
#include "SyntaxToken.h"
#include "SyntaxTree.h"
#include "VariableDeclarationSyntaxNode.h"

#include "Error/Record.h"

class Parser {
  std::vector<std::shared_ptr<SyntaxToken>> mTokens;
  int32_t mPosition;
  Records mRecords;
  std::shared_ptr<SyntaxToken> Peek(int32_t offset);
  std::shared_ptr<SyntaxToken> Current();
  std::shared_ptr<SyntaxToken> Next();
  std::shared_ptr<SyntaxToken> Match(SyntaxKind type);

  std::unique_ptr<ExpressionNode> ParseTerm();
  std::unique_ptr<ExpressionNode> ParseFactor();
  std::unique_ptr<ExpressionNode> ParsePrimaryExpression();
  std::unique_ptr<ExpressionNode> ParseAssignmentExpression();
  std::unique_ptr<ExpressionNode>
  ParseBinaryExpression(int parentPrecedence = 0);

  std::unique_ptr<StatementSyntaxNode> ParseStatement();
  std::unique_ptr<BlockStatementSyntaxNode> ParseBlockStatement();
  std::unique_ptr<StatementSyntaxNode> ParseVariableDeclaration();
  std::unique_ptr<ExpressionStatementSyntaxNode> ParseExpressionStatement();

public:
  Parser(std::string text);
  std::unique_ptr<CompilationUnitSyntaxNode> ParseCompilationUnit();
  const Records &Errors() const { return mRecords; }
};