/*
 * Copyright (c) 2022 F. Lotfi All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#pragma once
#include "ExpressionNode.h"
#include "CompilationUnitSyntaxNode.h"
#include <iostream>
#include <string>
#include <vector>

#include "Error/Record.h"

class SyntaxTree {
  Records mRecords;
  std::unique_ptr<CompilationUnitSyntaxNode> mRootExpression;
  void PrintTreeRec(SyntaxNode *sNode, std::ostream &out,
                    std::string indent = "", bool isLast = true);
  SyntaxTree(std::string text);
public:
  const Records &Errors() const { return mRecords; }
  CompilationUnitSyntaxNode *Root() const;
  void PrintTree(std::ostream &out = std::cout);
  static std::unique_ptr<SyntaxTree> Parse(std::string text);
};
