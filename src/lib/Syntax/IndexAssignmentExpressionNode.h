/*
 * Copyright (c) 2026 F. Lotfi All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#pragma once

#include "ExpressionNode.h"
#include "IndexExpressionNode.h"
#include "SyntaxToken.h"
#include <memory>

class IndexAssignmentExpressionNode : public ExpressionNode {
  std::unique_ptr<IndexExpressionNode> mIndexExpression;
  std::shared_ptr<SyntaxToken> mEqualsToken;
  std::unique_ptr<ExpressionNode> mValue;

public:
  IndexAssignmentExpressionNode(
      std::unique_ptr<IndexExpressionNode> indexExpression,
      std::shared_ptr<SyntaxToken> equalsToken,
      std::unique_ptr<ExpressionNode> value);
  IndexExpressionNode *IndexExpression() const;
  std::shared_ptr<SyntaxToken> EqualsToken() const;
  ExpressionNode *ValueExpression() const;
  virtual ~IndexAssignmentExpressionNode() {}
};
