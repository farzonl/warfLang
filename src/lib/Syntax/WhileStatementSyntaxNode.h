#pragma once

#include "ExpressionNode.h"
#include "StatementSyntaxNode.h"
#include "SyntaxToken.h"
#include <memory>

class WhileStatementSyntaxNode : public StatementSyntaxNode {
public:
  WhileStatementSyntaxNode(std::shared_ptr<SyntaxToken> whileKeyword,
                           std::shared_ptr<SyntaxToken> openParenthesisToken,
                           std::unique_ptr<ExpressionNode> condition,
                           std::shared_ptr<SyntaxToken> closeParenthesisToken,
                           std::unique_ptr<StatementSyntaxNode> body);

  std::unique_ptr<ExpressionNode> &Condition();
  const StatementSyntaxNode *Body() const;

private:
  std::shared_ptr<SyntaxToken> mWhileKeyword;
  std::shared_ptr<SyntaxToken> mOpenParenthesisToken;
  std::shared_ptr<SyntaxToken> mCloseParenthesisToken;
  std::unique_ptr<ExpressionNode> mCondition;
  std::unique_ptr<StatementSyntaxNode> mBody;
};
