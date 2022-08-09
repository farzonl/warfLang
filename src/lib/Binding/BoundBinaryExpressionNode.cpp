#include "BoundBinaryExpressionNode.h"

const BoundBinaryOperator BoundBinaryOperator::sOperators[] = {
      BoundBinaryOperator(SyntaxType::UnknownToken, BoundBinaryOperatorType::Addition, Type::Unknown, Type::Unknown),
      BoundBinaryOperator(SyntaxType::PlusToken, BoundBinaryOperatorType::Addition, Type::Number, Type::Number),
      BoundBinaryOperator(SyntaxType::MinusToken, BoundBinaryOperatorType::Subtraction, Type::Number, Type::Number),
      BoundBinaryOperator(SyntaxType::BangToken, BoundBinaryOperatorType::LogicalNegation, Type::Boolean, Type::Boolean),
      BoundBinaryOperator(SyntaxType::StarToken, BoundBinaryOperatorType::Multiplication, Type::Number, Type::Number),
      BoundBinaryOperator(SyntaxType::SlashToken, BoundBinaryOperatorType::Division, Type::Number, Type::Number),
    };

BoundBinaryOperator::BoundBinaryOperator(SyntaxType syntaxType, 
                                       BoundBinaryOperatorType boundType, 
                                    Type leftOperandType, 
                                    Type rightOperandType) :
                                    mSyntaxType(syntaxType),mBoundType(boundType),
                                    mLeftOperandType(leftOperandType),
                                    mRightOperandType(rightOperandType)
{
}

const BoundBinaryOperator& BoundBinaryOperator::GetBindFailure() {
    return sOperators[0];
}

const BoundBinaryOperator& BoundBinaryOperator::Bind(SyntaxType syntaxType, Type leftOperandType, Type rightOperandType) {
    for (auto op : sOperators) {
        if (op.GetSyntaxType() == syntaxType && op.OperandType() == leftOperandType && op.OperandType() == rightOperandType) {
            return op;
        }
    }
    return GetBindFailure();
}


BoundBinaryExpressionNode::BoundBinaryExpressionNode(std::unique_ptr<BoundExpressionNode> left, 
                                  const BoundBinaryOperator& op, 
                                  std::unique_ptr<BoundExpressionNode> right) : BoundExpressionNode(),
                                  mLeft(std::move(left)),
                                  mOperator(op),
                                  mRight(std::move(right)){}

BoundNodeType BoundBinaryExpressionNode::NodeType() {
    mLeft->NodeType();
}
Type BoundBinaryExpressionNode::GetType() {
    mLeft->GetType();
}

BoundExpressionNode* BoundBinaryExpressionNode::Left() {
    return mLeft.get();
}

BoundExpressionNode* BoundBinaryExpressionNode::Right() {
    return mRight.get();
}

BoundBinaryOperatorType BoundBinaryExpressionNode::OperatorType() {
    return mOperator;
}