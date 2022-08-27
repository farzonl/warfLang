/*
 * Copyright (c) 2022 F. Lotfi All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#pragma once

#include "BoundExpressionNode.h"
#include <string>

class BoundIdentifierExpressionNode : public BoundExpressionNode {
public:
  BoundIdentifierExpressionNode(const std::string &name);

  virtual BoundNodeType NodeType() override;
  virtual Type GetType() override;
  std::string Name() { return mName; }

private:
  std::string mName;
};
