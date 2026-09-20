#include "ForStatementSyntaxNode.h"

ForStatementSyntaxNode::ForStatementSyntaxNode(
    std::shared_ptr<SyntaxToken> forKeyword,
    std::shared_ptr<SyntaxToken> openParenthesisToken,
    std::unique_ptr<StatementSyntaxNode> initializer,
    std::shared_ptr<SyntaxToken> firstSemicolonToken,
    std::unique_ptr<ExpressionNode> condition,
    std::shared_ptr<SyntaxToken> secondSemicolonToken,
    std::unique_ptr<ExpressionNode> increment,
    std::shared_ptr<SyntaxToken> closeParenthesisToken,
    std::unique_ptr<StatementSyntaxNode> body)
    : StatementSyntaxNode(SyntaxKind::ForStatement),
      mForKeyword(std::move(forKeyword)),
      mOpenParenthesisToken(std::move(openParenthesisToken)),
      mFirstSemicolonToken(std::move(firstSemicolonToken)),
      mSecondSemicolonToken(std::move(secondSemicolonToken)),
      mCloseParenthesisToken(std::move(closeParenthesisToken)),
      mInitializer(std::move(initializer)), mCondition(std::move(condition)),
      mIncrement(std::move(increment)), mBody(std::move(body)) {
  mVecExpressionNodes = {mForKeyword.get(), mOpenParenthesisToken.get(),
                         mInitializer.get(), mFirstSemicolonToken.get(),
                         mCondition.get(), mSecondSemicolonToken.get(),
                         mIncrement.get(), mCloseParenthesisToken.get(),
                         mBody.get()};
}

const StatementSyntaxNode *ForStatementSyntaxNode::Initializer() const {
  return mInitializer.get();
}

std::unique_ptr<ExpressionNode> &ForStatementSyntaxNode::Condition() {
  return mCondition;
}

std::unique_ptr<ExpressionNode> &ForStatementSyntaxNode::Increment() {
  return mIncrement;
}

const StatementSyntaxNode *ForStatementSyntaxNode::Body() const {
  return mBody.get();
}
