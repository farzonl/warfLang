/*
 * Copyright (c) 2026 F. Lotfi All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#pragma once

#include "BoundExpressionNode.h"
#include "Symbol/VariableSymbol.h"
#include <memory>
#include <string>
#include <vector>

class BoundCallExpressionNode : public BoundExpressionNode {
public:
  BoundCallExpressionNode(
      std::string name,
      std::vector<std::unique_ptr<BoundExpressionNode>> arguments,
      std::vector<std::shared_ptr<VariableSymbol>> parameters,
      std::unique_ptr<BoundStatementNode> body, Value::Type type);
  virtual ~BoundCallExpressionNode() {}
  virtual BoundNodeKind Kind() override;
  virtual Value::Type Type() override;
  const std::string &Name() const;
  const std::vector<std::unique_ptr<BoundExpressionNode>> &Arguments() const;
  const std::vector<std::shared_ptr<VariableSymbol>> &Parameters() const;
  const BoundStatementNode *Body() const;

private:
  std::string mName;
  std::vector<std::unique_ptr<BoundExpressionNode>> mArguments;
  std::vector<std::shared_ptr<VariableSymbol>> mParameters;
  std::unique_ptr<BoundStatementNode> mBody;
  Value::Type mType;
};
