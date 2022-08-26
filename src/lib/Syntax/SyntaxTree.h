/*
 * Copyright (c) 2022 F. Lotfi All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#pragma once
#include "ExpressionNode.h"
#include <iostream>
#include <string>
#include <vector>

class SyntaxTree {
  std::vector<std::string> mVecErrors;
  std::unique_ptr<ExpressionNode> mRootExpression;
  void PrintTreeRec(SyntaxNode *sNode, std::ostream &out,
                    std::string indent = "", bool isLast = true);

public:
  SyntaxTree(std::vector<std::string> &vecErrors,
             std::unique_ptr<ExpressionNode> root);
  const std::vector<std::string> &Errors() const { return mVecErrors; }
  ExpressionNode *Root() const;
  void PrintTree(std::ostream &out = std::cout);
  static std::unique_ptr<SyntaxTree> Parse(std::string text);
};
