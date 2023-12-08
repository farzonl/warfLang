/*
 * Copyright (c) 2022 F. Lotfi All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#pragma once
#include <memory>
#include <vector>

#include "SyntaxKind.h"

class SyntaxNode {
protected:
  SyntaxKind mKind;
  std::vector<SyntaxNode *> mVecExpressionNodes;

public:
  SyntaxNode(SyntaxKind kind);
  SyntaxKind Kind() const { return mKind; }
  virtual const std::vector<SyntaxNode *> &GetChildren();
  virtual ~SyntaxNode() {}
};