// Copyright (c) 2022 F. Lotfi All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "Syntax/SyntaxTree.h"
#include <sstream>

void ParserFuzzTest(std::string &line) {
  auto syntaxTree = SyntaxTree::Parse(line);
  if (syntaxTree) {
    std::ostringstream out;
    syntaxTree->PrintTree(out);
  }
}

extern "C" int LLVMFuzzerTestOneInput(const char *cLine, size_t len) {
  if (cLine == nullptr) {
    return -1;
  }
  std::string sLine(cLine, len);
  try {
    ParserFuzzTest(sLine);
  } catch (...) {
    return -1;
  }
  return 0;
}
