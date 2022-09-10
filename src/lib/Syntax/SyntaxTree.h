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

#include "Error/Record.h"

class SyntaxTree {
  Records mRecords;
  std::unique_ptr<ExpressionNode> mRootExpression;
  void PrintTreeRec(SyntaxNode *sNode, std::ostream &out,
                    std::string indent = "", bool isLast = true);

public:
  SyntaxTree(Records &records, std::unique_ptr<ExpressionNode> root);
  const Records &Errors() const { return mRecords; }
  ExpressionNode *Root() const;
  void PrintTree(std::ostream &out = std::cout);
  static std::unique_ptr<SyntaxTree> Parse(std::string text);
};
