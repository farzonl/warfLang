// Copyright (c) 2026 F. Lotfi All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "IR/Ir.h"
#include "IR/IrOptimizer.h"

void IrParserFuzzTest(const std::string &line) {
  IrParser parser;
  auto module = parser.Parse(line);
  IrOptimizer::OptimizeInPlace(module);
}

extern "C" int LLVMFuzzerTestOneInput(const char *cLine, size_t len) {
  if (cLine == nullptr) {
    return -1;
  }
  std::string sLine(cLine, len);
  try {
    IrParserFuzzTest(sLine);
  } catch (...) {
    return -1;
  }
  return 0;
}
