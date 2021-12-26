#include "Syntax/Lexer.h"
#include "Syntax/Parser.h"

void LexerFuzzTest(std::string &line) {
  Lexer lex(line);
  while (lex.CurrentToken() != '\0') {
    lex.NextToken();
  }
}

void ParserFuzzTest(std::string &line) {
  Parser parser(line);
  parser.Parse();
}

extern "C" int LLVMFuzzerTestOneInput(const char *cLine, size_t len) {
  if (cLine == nullptr) {
    return -1;
  }
  std::string sLine(cLine, len);
  LexerFuzzTest(sLine);
  ParserFuzzTest(sLine);
  return 0;
}
