#include "BinaryExpressionNode.h"


BinaryExpressionNode::BinaryExpressionNode( std::unique_ptr<ExpressionNode> left, 
                              std::shared_ptr<SyntaxToken> operatorToken,
                              std::unique_ptr<ExpressionNode> right) : 
                              ExpressionNode(SyntaxType::BinaryExpression),
                              mLeft(std::move(left)),
                              mOperator(operatorToken),
                              mRight(std::move(right)) {
    mVecExpressionNodes.push_back(mLeft.get());
    mVecExpressionNodes.push_back(mOperator.get());
    mVecExpressionNodes.push_back(mRight.get());
}

ExpressionNode* BinaryExpressionNode::Left() {
    return mLeft.get();
}
ExpressionNode* BinaryExpressionNode::Right() {
    return mRight.get();
}
SyntaxToken* BinaryExpressionNode::Operator() {
    return mOperator.get();
}
