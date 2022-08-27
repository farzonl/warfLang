/*
 * Copyright (c) 2022 F. Lotfi All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#pragma once

#include "ExpressionNode.h"
#include "SyntaxToken.h"
#include <memory>

class IdentifierExpressionNode : public ExpressionNode {
private:
    std::shared_ptr<SyntaxToken> mIdentifier;
public:
  IdentifierExpressionNode(std::shared_ptr<SyntaxToken> identifierToken);
  std::shared_ptr<SyntaxToken> IdentifierToken();
  std::string Name() { return mIdentifier->Text();}
};