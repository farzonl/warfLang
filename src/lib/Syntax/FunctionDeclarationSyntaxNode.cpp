// Copyright (c) 2026 F. Lotfi All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "FunctionDeclarationSyntaxNode.h"

FunctionDeclarationSyntaxNode::FunctionDeclarationSyntaxNode(
    std::shared_ptr<SyntaxToken> functionKeyword,
    std::shared_ptr<SyntaxToken> identifier,
    std::shared_ptr<SyntaxToken> openParenthesis,
    std::vector<std::shared_ptr<SyntaxToken>> parameters,
    std::vector<std::shared_ptr<SyntaxToken>> commas,
    std::shared_ptr<SyntaxToken> closeParenthesis,
    std::unique_ptr<BlockStatementSyntaxNode> body,
    std::vector<Value::Type> parameterTypes, Value::Type returnType,
    std::shared_ptr<SyntaxToken> externKeyword)
    : StatementSyntaxNode(SyntaxKind::FunctionDeclaration),
      mExternKeyword(externKeyword), mFunctionKeyword(functionKeyword),
      mIdentifier(identifier), mOpenParenthesis(openParenthesis),
      mParameters(std::move(parameters)), mCommas(std::move(commas)),
      mCloseParenthesis(closeParenthesis), mBody(std::move(body)),
      mParameterTypes(std::move(parameterTypes)), mReturnType(returnType) {
  if (mExternKeyword) {
    mVecExpressionNodes.push_back(mExternKeyword.get());
  }
  mVecExpressionNodes.push_back(mFunctionKeyword.get());
  mVecExpressionNodes.push_back(mIdentifier.get());
  mVecExpressionNodes.push_back(mOpenParenthesis.get());
  for (size_t i = 0; i < mParameters.size(); i++) {
    mVecExpressionNodes.push_back(mParameters[i].get());
    if (i < mCommas.size()) {
      mVecExpressionNodes.push_back(mCommas[i].get());
    }
  }
  mVecExpressionNodes.push_back(mCloseParenthesis.get());
  if (mBody) {
    mVecExpressionNodes.push_back(mBody.get());
  }
}

std::shared_ptr<SyntaxToken> FunctionDeclarationSyntaxNode::Identifier() {
  return mIdentifier;
}

const std::vector<std::shared_ptr<SyntaxToken>> &
FunctionDeclarationSyntaxNode::Parameters() const {
  return mParameters;
}

const std::vector<Value::Type> &
FunctionDeclarationSyntaxNode::ParameterTypes() const {
  return mParameterTypes;
}

Value::Type FunctionDeclarationSyntaxNode::ReturnType() const {
  return mReturnType;
}

const BlockStatementSyntaxNode *FunctionDeclarationSyntaxNode::Body() const {
  return mBody.get();
}

bool FunctionDeclarationSyntaxNode::IsExtern() const {
  return mExternKeyword != nullptr;
}

std::shared_ptr<SyntaxToken>
FunctionDeclarationSyntaxNode::ExternKeyword() const {
  return mExternKeyword;
}
