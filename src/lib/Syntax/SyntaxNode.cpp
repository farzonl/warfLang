// Copyright (c) 2022 F. Lotfi All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "SyntaxNode.h"
#include "SyntaxToken.h"

SyntaxNode::SyntaxNode(SyntaxKind kind) : mKind(kind), mVecExpressionNodes() {}

const std::vector<SyntaxNode *> &SyntaxNode::GetChildren() {
  return mVecExpressionNodes;
}

TextSpan SyntaxNode::Span() const {
  int32_t minStart = -1;
  int32_t maxEnd = -1;

  for (const auto *child : mVecExpressionNodes) {
    if (!child)
      continue;
    TextSpan childSpan = child->Span();
    if (childSpan.Length() <= 0 && childSpan.Start() == 0 &&
        childSpan.End() == -1) {
      continue;
    }
    if (minStart == -1 || childSpan.Start() < minStart) {
      minStart = childSpan.Start();
    }
    int32_t childEndPos = childSpan.EndPosition();
    if (maxEnd == -1 || childEndPos > maxEnd) {
      maxEnd = childEndPos;
    }
  }

  if (minStart == -1) {
    return TextSpan(0, 0);
  }
  return TextSpan(minStart, maxEnd);
}

void SyntaxNode::CollectLeafTokens(
    std::vector<const SyntaxToken *> &leaves) const {
  if (auto *token = dynamic_cast<const SyntaxToken *>(this)) {
    leaves.push_back(token);
    return;
  }
  for (const auto *child : mVecExpressionNodes) {
    if (child) {
      child->CollectLeafTokens(leaves);
    }
  }
}

std::vector<const SyntaxToken *> SyntaxNode::GetLeafTokens() const {
  std::vector<const SyntaxToken *> leaves;
  CollectLeafTokens(leaves);
  return leaves;
}
