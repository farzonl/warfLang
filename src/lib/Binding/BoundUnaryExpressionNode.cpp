#include "BoundUnaryExpressionNode.h"

const BoundUnaryOperator BoundUnaryOperator::sOperators[] = {
    BoundUnaryOperator(SyntaxType::UnknownToken, BoundUnaryOperatorType::Identity, Type::Unknown),
        BoundUnaryOperator(SyntaxType::BangToken, BoundUnaryOperatorType::LogicalNegation, Type::Boolean),
        BoundUnaryOperator(SyntaxType::MinusToken, BoundUnaryOperatorType::Negation, Type::Number),
        BoundUnaryOperator(SyntaxType::PlusToken, BoundUnaryOperatorType::Identity, Type::Number),
    };

BoundUnaryOperator::BoundUnaryOperator(SyntaxType syntaxType, 
                                       BoundUnaryOperatorType unaryType, 
                                    Type operandValueType) :
                                    mSyntaxType(syntaxType),mUnaryType(unaryType),
                                    mOperandType(operandValueType)
{
}

const BoundUnaryOperator& BoundUnaryOperator::GetBindFailure() {
    return sOperators[0];
}

const BoundUnaryOperator& BoundUnaryOperator::Bind(SyntaxType syntaxType, Type operandType) {
    for (auto op : sOperators) {
        if (op.GetSyntaxType() == syntaxType && op.OperandType() == operandType) {
            return op;
        }
    }
    return GetBindFailure();
}

BoundUnaryExpressionNode::BoundUnaryExpressionNode(const BoundUnaryOperator& op, std::unique_ptr<BoundExpressionNode> operand) :
                          BoundExpressionNode(),
                          mOperator(op),
                          mOperand(std::move(operand))
{}

BoundNodeType BoundUnaryExpressionNode::NodeType() {
    return BoundNodeType::UnaryExpression;
}

Type BoundUnaryExpressionNode::GetType() {
    return mOperand->GetType();
}