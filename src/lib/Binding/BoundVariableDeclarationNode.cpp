/*
 * Copyright (c) 2022 F. Lotfi All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#include "BoundVariableDeclarationNode.h"

    BoundVariableDeclarationNode::BoundVariableDeclarationNode(std::shared_ptr<VariableSymbol> variable,
    std::unique_ptr<BoundExpressionNode> initializer): BoundStatementNode(), 
    mVariable(variable), 
    mInitializer(std::move(initializer)) {}

    BoundNodeKind BoundVariableDeclarationNode::Kind() {
      return BoundNodeKind::VariableDeclaration;
    }

    std::shared_ptr<VariableSymbol> BoundVariableDeclarationNode::Variable() const {
      return mVariable;
    }
    const BoundExpressionNode* BoundVariableDeclarationNode::Initializer() const {
      return mInitializer.get();
    }