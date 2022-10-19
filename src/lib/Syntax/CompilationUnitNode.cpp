#include "CompilationUnitNode.h"
#include "SyntaxType.h"

CompilationUnitNode::CompilationUnitNode(
    std::unique_ptr<ExpressionNode> expression)
    : SyntaxNode(SyntaxType::CompilationUnit),
      mExpression(std::move(expression)) {}

ExpressionNode *CompilationUnitNode::Expression() const {
  return mExpression.get();
}