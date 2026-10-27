/*
 * Copyright (c) 2026 F. Lotfi All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#pragma once

#include "StatementSyntaxNode.h"
#include "SyntaxToken.h"
#include <memory>

class ContinueStatementSyntaxNode : public StatementSyntaxNode {
public:
  explicit ContinueStatementSyntaxNode(
      std::shared_ptr<SyntaxToken> continueKeyword);
  const std::shared_ptr<SyntaxToken> &ContinueKeyword() const;

private:
  std::shared_ptr<SyntaxToken> mContinueKeyword;
};
