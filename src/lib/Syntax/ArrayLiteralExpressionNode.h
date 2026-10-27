/*
 * Copyright (c) 2026 F. Lotfi All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#pragma once

#include "ExpressionNode.h"
#include "SyntaxToken.h"
#include <memory>
#include <vector>

class ArrayLiteralExpressionNode : public ExpressionNode {
  std::shared_ptr<SyntaxToken> mOpenBracket;
  std::vector<std::unique_ptr<ExpressionNode>> mElements;
  std::vector<std::shared_ptr<SyntaxToken>> mCommas;
  std::shared_ptr<SyntaxToken> mCloseBracket;

public:
  ArrayLiteralExpressionNode(
      std::shared_ptr<SyntaxToken> openBracket,
      std::vector<std::unique_ptr<ExpressionNode>> elements,
      std::vector<std::shared_ptr<SyntaxToken>> commas,
      std::shared_ptr<SyntaxToken> closeBracket);
  std::shared_ptr<SyntaxToken> OpenBracket() const;
  const std::vector<std::unique_ptr<ExpressionNode>> &Elements() const;
  std::shared_ptr<SyntaxToken> CloseBracket() const;
  virtual ~ArrayLiteralExpressionNode() {}
};
