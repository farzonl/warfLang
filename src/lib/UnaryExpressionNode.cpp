#include "UnaryExpressionNode.h"

UnaryExpressionNode::UnaryExpressionNode(
    std::shared_ptr<SyntaxToken> operatorToken,
    std::unique_ptr<ExpressionNode> operand)
    : ExpressionNode(SyntaxType::UnaryExpression),
      mOperator(operatorToken), mOperand(std::move(operand)) {
  mVecExpressionNodes.push_back(mOperator.get());
  mVecExpressionNodes.push_back(mOperand.get());
}

ExpressionNode* UnaryExpressionNode::Operand() { return mOperand.get(); }
SyntaxToken* UnaryExpressionNode::Operator() { return mOperator.get(); }
