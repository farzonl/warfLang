#pragma once
#include "ExpressionNode.h"
#include <string>
#include <vector>

class SyntaxTree {
  std::vector<std::string> mVecErrors;
  std::unique_ptr<ExpressionNode> mRootExpression;
  void PrintTreeRec(SyntaxNode *sNode, std::string indent = "",
                    bool isLast = true);

public:
  SyntaxTree(std::vector<std::string> &vecErrors,
             std::unique_ptr<ExpressionNode> root);
  const std::vector<std::string> &Errors() const { return mVecErrors; }
  ExpressionNode *Root() const;
  void PrintTree();
  static std::unique_ptr<SyntaxTree> Parse(std::string text);
};
