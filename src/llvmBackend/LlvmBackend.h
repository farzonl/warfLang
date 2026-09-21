/*
 * Copyright (c) 2026 F. Lotfi All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#pragma once

#include "IR/Ir.h"
#include <string>

class LlvmBackend {
public:
  static std::string EmitLLVM(const IrModule &module,
                              bool emitExecutableMain = false);
  static bool WriteBitcode(const IrModule &module, const std::string &path,
                           bool emitExecutableMain = false);
};
