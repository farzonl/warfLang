#include "NumberNode.h"

NumberNode::NumberNode(std::shared_ptr<SyntaxToken> numberToken) : ExpressionNode(SyntaxType::NumberNode),
	mNumberToken(numberToken){
	mVecExpressionNodes.push_back(numberToken.get());
}
