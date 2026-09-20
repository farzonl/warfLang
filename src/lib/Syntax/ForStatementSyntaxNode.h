#pragma once

#include "ExpressionNode.h"
#include "StatementSyntaxNode.h"
#include "SyntaxToken.h"
#include <memory>

class ForStatementSyntaxNode : public StatementSyntaxNode {
public:
  ForStatementSyntaxNode(std::shared_ptr<SyntaxToken> forKeyword,
                         std::shared_ptr<SyntaxToken> openParenthesisToken,
                         std::unique_ptr<StatementSyntaxNode> initializer,
                         std::shared_ptr<SyntaxToken> firstSemicolonToken,
                         std::unique_ptr<ExpressionNode> condition,
                         std::shared_ptr<SyntaxToken> secondSemicolonToken,
                         std::unique_ptr<ExpressionNode> increment,
                         std::shared_ptr<SyntaxToken> closeParenthesisToken,
                         std::unique_ptr<StatementSyntaxNode> body);

  const StatementSyntaxNode *Initializer() const;
  std::unique_ptr<ExpressionNode> &Condition();
  std::unique_ptr<ExpressionNode> &Increment();
  const StatementSyntaxNode *Body() const;

private:
  std::shared_ptr<SyntaxToken> mForKeyword;
  std::shared_ptr<SyntaxToken> mOpenParenthesisToken;
  std::shared_ptr<SyntaxToken> mFirstSemicolonToken;
  std::shared_ptr<SyntaxToken> mSecondSemicolonToken;
  std::shared_ptr<SyntaxToken> mCloseParenthesisToken;
  std::unique_ptr<StatementSyntaxNode> mInitializer;
  std::unique_ptr<ExpressionNode> mCondition;
  std::unique_ptr<ExpressionNode> mIncrement;
  std::unique_ptr<StatementSyntaxNode> mBody;
};
