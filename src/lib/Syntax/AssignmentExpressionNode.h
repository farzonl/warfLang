/*
 * Copyright (c) 2022 F. Lotfi All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#pragma once

#include "ExpressionNode.h"
#include "SyntaxToken.h"
#include <memory>

class AssignmentExpressionNode : public ExpressionNode {
private:
  std::shared_ptr<SyntaxToken> mIdentifier;
  std::shared_ptr<SyntaxToken> mAssignmentOperator;
  std::unique_ptr<ExpressionNode> mExpression;

public:
  AssignmentExpressionNode(std::shared_ptr<SyntaxToken> identifierToken,
                           std::shared_ptr<SyntaxToken> assignmentToken,
                           std::unique_ptr<ExpressionNode> expression);

  std::shared_ptr<SyntaxToken> IdentifierToken();
  std::shared_ptr<SyntaxToken> AssignmentToken();
  ExpressionNode *Expression();
  virtual ~AssignmentExpressionNode() {}
};