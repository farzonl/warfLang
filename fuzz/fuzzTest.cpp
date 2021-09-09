#include "Lexer.h"
#include <cassert>

void LexerFuzzTest(std::string& line) {
    Lexer lex(line);
    while (lex.CurrentToken() != '\0') {
        auto sToken = lex.NextToken();
        assert(sToken->Text().size() > 0);
    }
}

extern "C" int LLVMFuzzerTestOneInput(const char *cLine) {
  if(cLine == nullptr) {
      return -1;
  }
  std::string sLine(cLine);
  LexerFuzzTest(sLine);
  return 0;
}