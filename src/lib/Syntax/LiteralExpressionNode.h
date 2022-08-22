/*
 * Copyright (c) 2022 F. Lotfi All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#pragma once
#include "ExpressionNode.h"
#include "SyntaxToken.h"

class LiteralExpressionNode : public ExpressionNode {
  std::shared_ptr<SyntaxToken> mLiteralToken;

public:
  LiteralExpressionNode(std::shared_ptr<SyntaxToken> literalToken);
  std::shared_ptr<SyntaxToken> LiteralToken() { return mLiteralToken; }
};