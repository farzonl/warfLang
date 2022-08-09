#pragma once
#include "BoundExpressionNode.h"
#include "Syntax/SyntaxType.h"

enum class BoundBinaryOperatorType {
    Addition,
    Subtraction,
    LogicalNegation,
    Multiplication,
    Division
};

class BoundBinaryOperator {
    public:
        static const BoundBinaryOperator& Bind(SyntaxType syntaxType, Type leftOperandType, Type rightOperandType);
        SyntaxType GetSyntaxType();
        BoundBinaryOperatorType BoundType();
        Type OperandType(); // expected Type
        Type EvalType();   // resulting Type
        static const BoundBinaryOperator& GetBindFailure();
    private:
        SyntaxType mSyntaxType;
        BoundBinaryOperatorType mBoundType;
        Type mLeftOperandType;
        Type mRightOperandType;
        Type mEvalType;
        BoundBinaryOperator(SyntaxType syntaxType, BoundBinaryOperatorType boundType, Type leftOperandType, Type rightOperandType);
        static const BoundBinaryOperator const sOperators[];
        BoundBinaryOperator() = delete;
        friend class BoundBinaryExpressionNode;
};

class BoundBinaryExpressionNode : public BoundExpressionNode {
    public:
        BoundBinaryExpressionNode(std::unique_ptr<BoundExpressionNode> left, 
                                  const BoundBinaryOperator& op, 
                                  std::unique_ptr<BoundExpressionNode> right);
        virtual BoundNodeType NodeType() override;
        virtual Type GetType() override; 
        BoundExpressionNode* Left();
        BoundExpressionNode* Right();
        BoundBinaryOperatorType OperatorType();
    private:
        std::unique_ptr<BoundExpressionNode> mLeft;
        std::unique_ptr<BoundExpressionNode> mRight;
        const BoundBinaryOperatorType& mOperator;
};