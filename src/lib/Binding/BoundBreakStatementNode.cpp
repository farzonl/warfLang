// Copyright (c) 2026 F. Lotfi All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "BoundBreakStatementNode.h"

BoundNodeKind BoundBreakStatementNode::Kind() {
  return BoundNodeKind::BreakStatement;
}
