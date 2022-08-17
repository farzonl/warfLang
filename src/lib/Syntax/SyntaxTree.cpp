#include "SyntaxTree.h"
#include "BinaryExpressionNode.h"
#include "LiteralExpressionNode.h"
#include "ParenthesizedExpressionNode.h"
#include "Parser.h"
#include "UnaryExpressionNode.h"
#include <iostream>

SyntaxTree::SyntaxTree(std::vector<std::string> &vecErrors,
                       std::unique_ptr<ExpressionNode> root)
    : mVecErrors(), mRootExpression(std::move(root)) {
  mVecErrors.swap(vecErrors);
}

ExpressionNode *SyntaxTree::Root() const { return mRootExpression.get(); }


std::unique_ptr<SyntaxTree> SyntaxTree::Parse(std::string text) {
  Parser parser(text);
  return parser.Parse();
}

void SyntaxTree::PrintTreeRec(SyntaxNode *sNode, std::string indent,
                              bool isLast) {
  std::string marker = isLast ? "L--" : "|--";
  std::cout << indent << marker << sNode->Type() << std::endl;
  SyntaxToken *derived = dynamic_cast<SyntaxToken *>(sNode);
  if (derived && derived->HasValue()) {
    std::cout << indent << "   " << derived->GetValue() << std::endl;
  }

  indent += isLast ? "    " : "|   ";
  auto vecChildren = sNode->GetChildren();
  for (auto child : vecChildren) {
    bool isLastChild = (child == vecChildren[vecChildren.size() - 1]);
    PrintTreeRec(child, indent, isLastChild);
  }
}

void SyntaxTree::PrintTree() { PrintTreeRec(this->mRootExpression.get()); }