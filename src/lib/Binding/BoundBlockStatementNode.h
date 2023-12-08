/*
 * Copyright (c) 2022 F. Lotfi All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#pragma once

#include "BoundExpressionNode.h"
#include "DataStructures/TemplateList.h"


class BoundBlockStatementNode : public BoundStatementNode {
  public:
    BoundBlockStatementNode(TemplateList<std::unique_ptr<BoundStatementNode>> statements);
  
    virtual BoundNodeKind Kind() override;
    const TemplateList<std::unique_ptr<BoundStatementNode>>& Statements() const;
  private:
    TemplateList<std::unique_ptr<BoundStatementNode>> mStatements;
};