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

class CallExpressionNode : public ExpressionNode {
  std::shared_ptr<SyntaxToken> mIdentifier;
  std::shared_ptr<SyntaxToken> mOpenParenthesis;
  std::vector<std::unique_ptr<ExpressionNode>> mArguments;
  std::vector<std::shared_ptr<SyntaxToken>> mCommas;
  std::shared_ptr<SyntaxToken> mCloseParenthesis;

public:
  CallExpressionNode(std::shared_ptr<SyntaxToken> identifier,
                     std::shared_ptr<SyntaxToken> openParenthesis,
                     std::vector<std::unique_ptr<ExpressionNode>> arguments,
                     std::vector<std::shared_ptr<SyntaxToken>> commas,
                     std::shared_ptr<SyntaxToken> closeParenthesis);
  std::shared_ptr<SyntaxToken> IdentifierToken();
  const std::vector<std::unique_ptr<ExpressionNode>> &Arguments() const;
  virtual ~CallExpressionNode() {}
};
