// Copyright (c) 2022 F. Lotfi All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "Binding/Binder.h"
#include "CodeAnalysis/Evaluator.h"
#include "ExpressionStatementSyntaxNode.h"
#include "Symbol/SymbolTableMgr.h"
#include "Syntax/SyntaxTree.h"
#include "Version/version.h"
#include <fstream>
#include <functional>
#include <sstream>

#if !defined(_WIN32) && !defined(__wasm) && !defined(DISABLE_LIBEDIT)
#include <editline/readline.h>
#endif
#if !defined(_WIN32)
#include <unistd.h>
#endif

// Cap readline/libedit history so a long-running REPL can't grow it unbounded.
static const int kMaxReplHistoryEntries = 1000;

struct Flags {
  struct FlagName {
    const std::string shortName;
    const std::string name;
  };
  static const FlagName help;
  static const FlagName eval;
  static const FlagName repl;
  static const FlagName showTree;
  Flags() = delete;
};

class ParseFile {
public:
  ParseFile(const std::string &path,
            std::function<void(std::string &, bool, std::stringstream &)>
                parseLineBehavior);
  bool parse(bool showTree);

private:
  // TODO make parser handle multiple files
  // std::vector<std::string> mInputFilePaths;
  std::string mInputFilePath;
  std::function<void(std::string &, bool, std::stringstream &)>
      mParseLineBehavior;
};

ParseFile::ParseFile(
    const std::string &path,
    std::function<void(std::string &, bool, std::stringstream &)>
        parseLineBehavior)
    : mInputFilePath(path), mParseLineBehavior(parseLineBehavior) {}

// Braces can span multiple physical lines (e.g. a block statement), so we
// need to know how many are still unclosed to decide whether a statement is
// complete. Text after "//" is a comment and must not affect the count.
static int32_t CountUnmatchedBraces(const std::string &line) {
  int32_t delta = 0;
  for (size_t i = 0; i < line.size(); i++) {
    if (line[i] == '/' && i + 1 < line.size() && line[i + 1] == '/') {
      break;
    }
    if (line[i] == '{') {
      delta++;
    } else if (line[i] == '}') {
      delta--;
    }
  }
  return delta;
}

bool ParseFile::parse(bool showTree) {
  std::ifstream file(mInputFilePath);
  if (!file.is_open()) {
    // Error couldn't open the file
    return false;
  }

  std::string line;
  std::stringstream textBlock;
  std::string statement;
  int32_t braceDepth = 0;
  while (getline(file, line)) {
    braceDepth += CountUnmatchedBraces(line);
    if (!statement.empty()) {
      statement += " ";
    }
    statement += line;
    if (braceDepth <= 0) {
      mParseLineBehavior(statement, showTree, textBlock);
      statement.clear();
      braceDepth = 0;
    }
  }
  if (!statement.empty()) {
    mParseLineBehavior(statement, showTree, textBlock);
  }

  file.close();
  return file.eof() && !file.bad();
}

const Flags::FlagName Flags::help = {"-h", "--help"};
const Flags::FlagName Flags::eval = {"-e", "--eval"};
const Flags::FlagName Flags::repl = {"-r", "--repl"};
const Flags::FlagName Flags::showTree = {"-s", "--show_tree"};

void printUsage() {
  std::cout << "usage: ./Warf \n";
  std::cout << "       ./Warf <file_name>\n";
  std::cout << "       ./Warf <file_name> --show_tree\n";
  std::cout << "       ./Warf --repl\n";
  std::cout << "       ./Warf --eval <string_to_evaluate>\n";
  std::cout << "       ./Warf --eval <string_to_evaluate> --show_tree\n";
  std::cout << "       ./Warf -h\n";
  std::cout << "       ./Warf --help\n";
}

ExpressionNode *ParseExpression(SyntaxTree *syntaxTree) {
  auto root = syntaxTree->Root();
  auto statement = root->Statement();
  if (statement->Kind() == SyntaxKind::ExpressionStatement) {
    auto expressionStatement = dynamic_cast<ExpressionStatementSyntaxNode *>(
        const_cast<StatementSyntaxNode *>(statement));
    return const_cast<ExpressionNode *>(expressionStatement->Expression());
  }
  return nullptr;
}

void evaluate(std::string &line, bool showTree, std::stringstream &textBlock) {
  auto globalScope = SymbolTableMgr::getGlobalScope();
  std::string source = line;
  auto syntaxTree = SyntaxTree::Parse(source);

  // Comment-only/blank lines parse to zero statements; nothing to bind, show,
  // or evaluate.
  if (syntaxTree->Root()->Statements().empty()) {
    return;
  }
  if (syntaxTree->Errors().empty() &&
      syntaxTree->Root()->Statements().size() == 1 &&
      syntaxTree->Root()->Statement()->Kind() ==
          SyntaxKind::FunctionDeclaration) {
    textBlock << line << " ";
    return;
  }
  if (!textBlock.str().empty()) {
    source = textBlock.str() + line;
    syntaxTree = SyntaxTree::Parse(source);
  }
  globalScope->GetTextSpan()->updateTextSpan(0, source.size());
  auto binder = std::make_unique<Binder>();
  std::unique_ptr<BoundStatementNode> boundStatement;
  try {
    boundStatement = binder->BindCompilationUnit(syntaxTree->Root());
  } catch (std::runtime_error &error) {
    std::cerr << error.what() << std::endl;
  }

  if (showTree) {
    syntaxTree->PrintTree();
  }

  if (syntaxTree->Errors().empty() && binder->Errors().empty()) {
    auto eval = std::make_unique<Evaluator>(std::move(boundStatement));
    Value result = eval->Evaluate();
    if (result.VType() != Value::Type::Unknown) {
      std::cout << result << std::endl;
    }
  } else {
    for (auto error : syntaxTree->Errors()) {
      std::cerr << error << std::endl;
    }
    for (auto error : binder->Errors()) {
      std::cerr << error << std::endl;
    }
  }
}

void consoleRead(bool &showTree, std::stringstream &textBlock) {
#if !defined(_WIN32) && !defined(__wasm) && !defined(DISABLE_LIBEDIT)
  char *buffer = readline(">>> ");

  if (buffer == nullptr) {
    exit(0);
  }

  // Add input history
  if (buffer[0] != '\0') {
    add_history(buffer);
  }

  std::string line(buffer);
  free(buffer);
#else
  std::string line = "";
  std::cout << ">>> ";
  if (!std::getline(std::cin, line)) {
    exit(0);
  }
#endif
  if (line == "#showTree") {
    showTree = !showTree;
    std::cout << (showTree ? "Showing parse trees." : "Not showing parse trees")
              << std::endl;
    return;
  }
  if (line == "#exit") {
    exit(0);
  }
  evaluate(line, showTree, textBlock);
}

void startRepl(bool showTree) {
#if !defined(_WIN32) && !defined(__wasm) && !defined(DISABLE_LIBEDIT)
  stifle_history(kMaxReplHistoryEntries);
#endif
  WarfHelper::printVersion();
  std::stringstream textBlock;
  while (true) {
    try {
      consoleRead(showTree, textBlock);

    } catch (std::exception &error) {
      std::cerr << error.what() << std::endl;
      std::cin.get();
    }
  }
}

int main(int argc, char **argv) {
  SymbolTableMgr::init();
  bool isEval = false;
  bool isRepl = false;
  bool showTree = false;
  int showTreeIndex = 0;
  std::string evalStr;
  for (int i = 1; i < argc; i++) {
    if (Flags::help.shortName == argv[i] || Flags::help.name == argv[i]) {
      printUsage();
      return 0;
    }
    if (Flags::eval.shortName == argv[i] || Flags::eval.name == argv[i]) {
      isEval = true;
      if (i + 1 >= argc) {
        std::cerr << "Expected eval argument string" << std::endl;
        return -1;
      }
      evalStr = std::string(argv[i + 1]);
    } else if (Flags::showTree.shortName == argv[i] ||
               Flags::showTree.name == argv[i]) {
      showTree = true;
      showTreeIndex = i;
    } else if (Flags::repl.shortName == argv[i] ||
               Flags::repl.name == argv[i]) {
      isRepl = true;
    } else if (argv[i][0] == '-') {
      std::cerr << "Invalid flag: " << argv[i] << std::endl;
      printUsage();
      return 0;
    }
  }

  if (isEval) {
    // TODO we broke one line evaluate
    std::stringstream textBlock;
    evaluate(evalStr, showTree, textBlock);
    return 0;
  }
  if (isRepl || argc == 1 || (showTree && argc == 2)) {
#if !defined(_WIN32)
    // Without a real interactive terminal, readline() can't block on input
    // the way the REPL expects; refuse to spin one up unattended.
    if (!isatty(fileno(stdin))) {
      std::cerr << "stdin is not a terminal; refusing to start the REPL."
                << std::endl;
      printUsage();
      return -1;
    }
#endif
    startRepl(showTree);
    return 0;
  }
  if (argc > 3) {
    printUsage();
    return 0;
  }

  int filePathIndex = 1 == showTreeIndex ? 2 : 1;
  std::string filePath(argv[filePathIndex]);
  ParseFile file(filePath, evaluate);
  file.parse(showTree);
  return 0;
}
