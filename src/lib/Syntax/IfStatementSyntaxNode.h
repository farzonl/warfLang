#pragma once

#include "ExpressionNode.h"
#include "StatementSyntaxNode.h"
#include "SyntaxToken.h"
#include <memory>

class IfStatementSyntaxNode : public StatementSyntaxNode {
public:
  IfStatementSyntaxNode(std::shared_ptr<SyntaxToken> ifKeyword,
                        std::shared_ptr<SyntaxToken> openParenthesisToken,
                        std::unique_ptr<ExpressionNode> condition,
                        std::shared_ptr<SyntaxToken> closeParenthesisToken,
                        std::unique_ptr<StatementSyntaxNode> thenStatement,
                        std::shared_ptr<SyntaxToken> elseKeyword,
                        std::unique_ptr<StatementSyntaxNode> elseStatement);

  std::unique_ptr<ExpressionNode> &Condition();
  const StatementSyntaxNode *ThenStatement() const;
  const StatementSyntaxNode *ElseStatement() const;

private:
  std::shared_ptr<SyntaxToken> mIfKeyword;
  std::shared_ptr<SyntaxToken> mOpenParenthesisToken;
  std::shared_ptr<SyntaxToken> mCloseParenthesisToken;
  std::unique_ptr<ExpressionNode> mCondition;
  std::unique_ptr<StatementSyntaxNode> mThenStatement;
  std::shared_ptr<SyntaxToken> mElseKeyword;
  std::unique_ptr<StatementSyntaxNode> mElseStatement;
};
