#pragma once
#include "BoundExpressionNode.h"
#include "Syntax/SyntaxType.h"

enum class BoundBinaryOperatorType {
    Addition,
    Subtraction,
    LogicalNegation,
    Multiplication,
    Division,
    EqualsEquals,
    BangEquals
};

static const std::unordered_map<BoundBinaryOperatorType, std::string> BoundBinaryTypeStrMap = {
    {BoundBinaryOperatorType::Addition, "Addition"},
    {BoundBinaryOperatorType::Subtraction, "Subtraction"},
    {BoundBinaryOperatorType::LogicalNegation, "LogicalNegation"},
    {BoundBinaryOperatorType::Multiplication, "Multiplication"},
    {BoundBinaryOperatorType::Division, "Division"},
    {BoundBinaryOperatorType::EqualsEquals, "EqualsEquals"},
    {BoundBinaryOperatorType::BangEquals, "BangEquals"},
};

class BoundBinaryOperator {
    public:
        static const std::shared_ptr<BoundBinaryOperator> Bind(SyntaxType syntaxType, Type leftOperandType, Type rightOperandType);
        SyntaxType GetSyntaxType();
        BoundBinaryOperatorType BoundType();
        Type LeftOperandType(); // expected Type
        Type RightOperandType(); // expected Type
        Type EvalType();   // resulting Type
        static const std::shared_ptr<BoundBinaryOperator> GetBindFailure();
        //TODO see if there is a way to make shared_ptr a friend function
        BoundBinaryOperator(SyntaxType syntaxType, BoundBinaryOperatorType boundType, Type leftOperandType, Type rightOperandType);
    private:
        SyntaxType mSyntaxType;
        BoundBinaryOperatorType mBoundType;
        Type mLeftOperandType;
        Type mRightOperandType;
        Type mEvalType;
        static const std::shared_ptr<BoundBinaryOperator> sOperators[];
        BoundBinaryOperator() = delete;
        friend class BoundBinaryExpressionNode;
};

class BoundBinaryExpressionNode : public BoundExpressionNode {
    public:
        BoundBinaryExpressionNode(std::unique_ptr<BoundExpressionNode> left, 
                                  const std::shared_ptr<BoundBinaryOperator> op, 
                                  std::unique_ptr<BoundExpressionNode> right);
        virtual BoundNodeType NodeType() override;
        virtual Type GetType() override; 
        BoundExpressionNode* Left();
        BoundExpressionNode* Right();
        BoundBinaryOperatorType OperatorType();
    private:
        std::unique_ptr<BoundExpressionNode> mLeft;
        std::unique_ptr<BoundExpressionNode> mRight;
        const std::shared_ptr<BoundBinaryOperator> mOperator;
};