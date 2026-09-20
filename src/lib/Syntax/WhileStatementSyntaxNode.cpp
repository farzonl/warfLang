#include "WhileStatementSyntaxNode.h"

WhileStatementSyntaxNode::WhileStatementSyntaxNode(
    std::shared_ptr<SyntaxToken> whileKeyword,
    std::shared_ptr<SyntaxToken> openParenthesisToken,
    std::unique_ptr<ExpressionNode> condition,
    std::shared_ptr<SyntaxToken> closeParenthesisToken,
    std::unique_ptr<StatementSyntaxNode> body)
    : StatementSyntaxNode(SyntaxKind::WhileStatement),
      mWhileKeyword(std::move(whileKeyword)),
      mOpenParenthesisToken(std::move(openParenthesisToken)),
      mCloseParenthesisToken(std::move(closeParenthesisToken)),
      mCondition(std::move(condition)), mBody(std::move(body)) {
  mVecExpressionNodes = {mWhileKeyword.get(), mOpenParenthesisToken.get(),
                         mCondition.get(), mCloseParenthesisToken.get(),
                         mBody.get()};
}

std::unique_ptr<ExpressionNode> &WhileStatementSyntaxNode::Condition() {
  return mCondition;
}

const StatementSyntaxNode *WhileStatementSyntaxNode::Body() const {
  return mBody.get();
}
