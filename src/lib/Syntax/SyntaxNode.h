/*
 * Copyright (c) 2022 F. Lotfi All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#pragma once
#include <memory>
#include <vector>

#include "SyntaxType.h"

class SyntaxNode {
protected:
  SyntaxType mType;
  std::vector<SyntaxNode *> mVecExpressionNodes;

public:
  SyntaxNode(SyntaxType type);
  SyntaxType Type() { return mType; }
  virtual const std::vector<SyntaxNode *> &GetChildren();
};