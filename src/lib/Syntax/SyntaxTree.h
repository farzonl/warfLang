/*
 * Copyright (c) 2022 F. Lotfi All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#pragma once
#include "ExpressionNode.h"
#include "CompilationUnitNode.h"
#include <iostream>
#include <string>
#include <vector>

#include "Error/Record.h"

class SyntaxTree {
  Records mRecords;
  SyntaxTree(std::string text);
  std::unique_ptr<CompilationUnitNode> mRootExpression;
  void PrintTreeRec(SyntaxNode *sNode, std::ostream &out,
                    std::string indent = "", bool isLast = true);

public:
  const Records &Errors() const { return mRecords; }
  CompilationUnitNode *Root() const;
  void PrintTree(std::ostream &out = std::cout);
  static std::unique_ptr<SyntaxTree> Parse(std::string text);
  friend std::unique_ptr<SyntaxTree> std::make_unique<SyntaxTree>(std::string&);
  friend std::unique_ptr<SyntaxTree> std::make_unique<SyntaxTree>(std::string&&);

};
