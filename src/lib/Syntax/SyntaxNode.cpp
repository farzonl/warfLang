// Copyright (c) 2022 F. Lotfi All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "SyntaxNode.h"

SyntaxNode::SyntaxNode(SyntaxType type) : mType(type), mVecExpressionNodes() {}

const std::vector<SyntaxNode *> &SyntaxNode::GetChildren() {
  return mVecExpressionNodes;
}