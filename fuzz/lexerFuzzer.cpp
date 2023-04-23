// Copyright (c) 2022 F. Lotfi All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "Syntax/Lexer.h"

void LexerFuzzTest(std::string &line) {
  Lexer lex(line);
  while (lex.CurrentToken() != '\0') {
    lex.NextToken();
  }
}

extern "C" int LLVMFuzzerTestOneInput(const char *cLine, size_t len) {
  if (cLine == nullptr) {
    return -1;
  }
  std::string sLine(cLine, len);
  try {
    LexerFuzzTest(sLine);
  } catch (...) {
    return -1;
  }
  return 0;
}
