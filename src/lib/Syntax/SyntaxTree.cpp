// Copyright (c) 2022 F. Lotfi All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "SyntaxTree.h"
#include "BinaryExpressionNode.h"
#include "LiteralExpressionNode.h"
#include "ParenthesizedExpressionNode.h"
#include "Parser.h"
#include "UnaryExpressionNode.h"

SyntaxTree::SyntaxTree(std::string text)
    : mRecords(""), mRootExpression(nullptr) {
      Parser parser(text);
      mRootExpression = parser.ParseCompilationUnit();
      mRecords.swap(parser.Errors()); 
}

CompilationUnitSyntaxNode *SyntaxTree::Root() const { return mRootExpression.get(); }

std::unique_ptr<SyntaxTree> SyntaxTree::Parse(std::string text) {
   return std::unique_ptr<SyntaxTree>(new SyntaxTree(text));
}

void SyntaxTree::PrintTreeRec(SyntaxNode *sNode, std::ostream &out,
                              std::string indent, bool isLast) {
  std::string marker = isLast ? "L--" : "|--";
  out << indent << marker << sNode->Kind() << std::endl;
  SyntaxToken *derived = dynamic_cast<SyntaxToken *>(sNode);
  if (derived && derived->Kind() == SyntaxKind::IdentifierToken) {
    out << indent << "   " << derived->Text() << std::endl;
  }
  if (derived && derived->HasValue()) {
    out << indent << "   " << derived->GetValue() << std::endl;
  }

  indent += isLast ? "    " : "|   ";
  auto vecChildren = sNode->GetChildren();
  for (auto child : vecChildren) {
    bool isLastChild = (child == vecChildren[vecChildren.size() - 1]);
    PrintTreeRec(child, out, indent, isLastChild);
  }
}

void SyntaxTree::PrintTree(std::ostream &out) {
  PrintTreeRec(this->mRootExpression.get(), out);
}