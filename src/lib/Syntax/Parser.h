/*
 * Copyright (c) 2022 F. Lotfi All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#pragma once
#include <memory>

#include "ExpressionNode.h"
#include "SyntaxToken.h"
#include "SyntaxTree.h"
#include "CompilationUnitNode.h"
#include "Error/Record.h"

class Parser {
  std::vector<std::shared_ptr<SyntaxToken>> mTokens;
  int32_t mPosition;
  Records mRecords;
  std::shared_ptr<SyntaxToken> Peek(int32_t offset);
  std::shared_ptr<SyntaxToken> Current();
  std::shared_ptr<SyntaxToken> Next();
  std::shared_ptr<SyntaxToken> Match(SyntaxType type);

  std::unique_ptr<ExpressionNode> ParseTerm();
  std::unique_ptr<ExpressionNode> ParseFactor();
  std::unique_ptr<ExpressionNode> ParsePrimaryExpression();
  std::unique_ptr<ExpressionNode> ParseAssignmentExpression();
  std::unique_ptr<ExpressionNode>
  ParseBinaryExpression(int parentPrecedence = 0);

public:
  Parser(std::string text);
  std::unique_ptr<CompilationUnitNode> Parse();
  const Records &Errors() const { return mRecords; }
};