/*
 * Copyright (c) 2026 F. Lotfi All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#pragma once

#include "StatementSyntaxNode.h"
#include "SyntaxToken.h"
#include <memory>

class BreakStatementSyntaxNode : public StatementSyntaxNode {
public:
  explicit BreakStatementSyntaxNode(std::shared_ptr<SyntaxToken> breakKeyword);
  const std::shared_ptr<SyntaxToken> &BreakKeyword() const;

private:
  std::shared_ptr<SyntaxToken> mBreakKeyword;
};
