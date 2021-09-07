#include "SyntaxTree.h"
#include "NumberNode.h"
#include "BinaryExpressionNode.h"
#include "ParenthesizedExpressionNode.h"
#include "Parser.h"
#include <iostream>

SyntaxTree::SyntaxTree(std::vector<std::string> &vecErrors, std::unique_ptr<ExpressionNode> root) :
             mVecErrors(), mRootExpression(std::move(root)) {
    mVecErrors.swap(vecErrors);
}

ExpressionNode* SyntaxTree::Root() const{
    return mRootExpression.get();
}

int SyntaxTree::EvaluateRec(ExpressionNode* node) {
    if(NumberNode* number = dynamic_cast<NumberNode*>(node)) {
        return number->NumberToken()->HasValue() ?  number->NumberToken()->Value().asInt() : 0;
    }
    if(NumberNode* number = dynamic_cast<NumberNode*>(node)) {
        return number->NumberToken()->HasValue() ?  number->NumberToken()->Value().asInt() : 0;
    }

    if(BinaryExpressionNode* binaryExpression = dynamic_cast<BinaryExpressionNode*>(node)) {
        auto left  = EvaluateRec(binaryExpression->Left());
        auto right = EvaluateRec(binaryExpression->Right());
        auto opType = binaryExpression->Operator()->Type();
        switch(opType) {
            case SyntaxType::PlusToken:
                return left + right;
            case SyntaxType::MinusToken :
                return left - right;
            case SyntaxType::StarToken:
                return left * right;
            case SyntaxType::SlashToken :
                return left / right;
            default:
                 mVecErrors.push_back("EvaluatorError: Unexpected binary operator: " + SyntaxTokenToStrMap.at(opType));
                return 0;
        }
    }
     if(ParenthesizedExpressionNode* parenExpression = dynamic_cast<ParenthesizedExpressionNode*>(node)) {
         return EvaluateRec(parenExpression->Expression());
     }

     mVecErrors.push_back("EvaluatorError: Unexpected node: " + SyntaxTypeStrMap.at(node->Type()));
     return 0;
}

int SyntaxTree::Evaluate() {
    return EvaluateRec(mRootExpression.get());
}

std::unique_ptr <SyntaxTree> SyntaxTree::Parse(std::string text) {
    Parser parser(text);
    return parser.Parse();
}

void SyntaxTree::PrintTreeRec(SyntaxNode* sNode, std::string indent, bool isLast) {
    std::string  marker = isLast ? "L--" : "|--";
    std::cout << indent << marker << sNode->Type() << std::endl;
    SyntaxToken* derived = dynamic_cast<SyntaxToken*>(sNode);
    if (derived && derived->HasValue()) {
        std::cout << indent << "   " << derived->Value() << std::endl;
    }

    indent += isLast ? "    " : "|   ";
    auto vecChildren = sNode->GetChildren();
    for (auto child : vecChildren) {
        bool isLastChild = (child == vecChildren[vecChildren.size() - 1]);
        PrintTreeRec(child, indent, isLastChild);
    }
}

void SyntaxTree::PrintTree() {
    PrintTreeRec(this->mRootExpression.get());
}