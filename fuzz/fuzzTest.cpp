#include "Lexer.h"
#include <cassert>

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
  LexerFuzzTest(sLine);
  return 0;
}