#include "SyntaxNode.h"

SyntaxNode::SyntaxNode(SyntaxType type) : mType(type), mVecExpressionNodes() {}

const std::vector<SyntaxNode*>& SyntaxNode::GetChildren() {
	return mVecExpressionNodes;
}