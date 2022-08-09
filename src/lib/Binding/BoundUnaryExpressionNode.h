#pragma once

#include <memory>

#include "BoundExpressionNode.h"
#include "Syntax/SyntaxType.h"
#include "ValueType.h"

enum class BoundUnaryOperatorType {
    Identity,
    Negation,
    LogicalNegation,
};

class BoundUnaryOperator {
    public:
        static const BoundUnaryOperator& Bind(SyntaxType syntaxType, Type operandType);
        SyntaxType GetSyntaxType();
        BoundUnaryOperatorType UnaryType();
        Type OperandType(); // expected Type
        Type EvalType();   // resulting Type
        static const BoundUnaryOperator& GetBindFailure();
    private:
        SyntaxType mSyntaxType;
        BoundUnaryOperatorType mUnaryType;
        Type mOperandType;
        Type mEvalType;
        BoundUnaryOperator(SyntaxType syntaxType, BoundUnaryOperatorType unaryType, Type operandValueType);
        static const BoundUnaryOperator const sOperators[];
        BoundUnaryOperator() = delete;
        friend class BoundUnaryExpressionNode;
};

class BoundUnaryExpressionNode : public BoundExpressionNode {
    public:
        BoundUnaryExpressionNode(const BoundUnaryOperator& op, std::unique_ptr<BoundExpressionNode> operand);
        virtual BoundNodeType NodeType() override;
        virtual Type GetType() override;
        BoundExpressionNode* Operand();
        virtual ~BoundUnaryExpressionNode() {}
    private:
        const BoundUnaryOperator& mOperator;
        std::unique_ptr<BoundExpressionNode> mOperand;
};