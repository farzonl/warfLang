#pragma once
#include "ExpressionNode.h"
#include "SyntaxToken.h"

class UnaryExpressionNode : public ExpressionNode {
  std::unique_ptr<ExpressionNode> mOperand;
  std::shared_ptr<SyntaxToken> mOperator;

public:
  UnaryExpressionNode(std::shared_ptr<SyntaxToken> operatorToken,
                      std::unique_ptr<ExpressionNode> operand);
  
  ExpressionNode *Operand();
  SyntaxToken *Operator();
};