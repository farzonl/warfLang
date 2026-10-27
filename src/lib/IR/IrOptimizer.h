/*
 * Copyright (c) 2026 F. Lotfi All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#pragma once

#include "IR/Ir.h"

class IrOptimizer {
public:
  static IrModule Optimize(const IrModule &module);
  static void OptimizeInPlace(IrModule &module);
};
