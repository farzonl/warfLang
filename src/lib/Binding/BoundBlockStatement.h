/*
 * Copyright (c) 2022 F. Lotfi All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#pragma once

#include "BoundStatement.h"
#include <vector>

class BoundBlockStatement : public BoundStatement {
  std::vector<std::unique_ptr<BoundStatement>> mStatements;

public:
  BoundBlockStatement(std::vector<std::unique_ptr<BoundStatement>> &statements);
  const std::vector<std::unique_ptr<BoundStatement>> &Statements() const;
  virtual BoundNodeType NodeType() override;
};