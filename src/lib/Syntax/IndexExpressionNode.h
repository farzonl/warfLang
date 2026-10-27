/*
 * Copyright (c) 2026 F. Lotfi All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#pragma once

#include "ExpressionNode.h"
#include "SyntaxToken.h"
#include <memory>

class IndexExpressionNode : public ExpressionNode {
  std::unique_ptr<ExpressionNode> mTarget;
  std::shared_ptr<SyntaxToken> mOpenBracket;
  std::unique_ptr<ExpressionNode> mIndex;
  std::shared_ptr<SyntaxToken> mCloseBracket;

public:
  IndexExpressionNode(std::unique_ptr<ExpressionNode> target,
                      std::shared_ptr<SyntaxToken> openBracket,
                      std::unique_ptr<ExpressionNode> index,
                      std::shared_ptr<SyntaxToken> closeBracket);
  ExpressionNode *Target() const;
  std::shared_ptr<SyntaxToken> OpenBracket() const;
  ExpressionNode *Index() const;
  std::shared_ptr<SyntaxToken> CloseBracket() const;
  virtual ~IndexExpressionNode() {}
};
