// Copyright (c) 2022 F. Lotfi All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "Binding/Binder.h"
#include "Evaluator.h"
#include "Symbol/SymbolTableMgr.h"
#include "Syntax/SyntaxTree.h"
#include "Version/version.h"
#include <fstream>
#include <functional>

#if !defined(_WIN32) && !defined(__wasm) && !defined(DISABLE_LIBEDIT)
#include <editline/readline.h>
#endif

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
            std::function<void(std::string &, bool)> parseLineBehavior);
  bool parse(bool showTree);

private:
  // TODO make parser hanle multiple files
  // std::vector<std::string> mInputFilePaths;
  std::string mInputFilePath;
  std::function<void(std::string &, bool)> mParseLineBehavior;
};

ParseFile::ParseFile(const std::string &path,
                     std::function<void(std::string &, bool)> parseLineBehavior)
    : mInputFilePath(path), mParseLineBehavior(parseLineBehavior) {}

bool ParseFile::parse(bool showTree) {
  std::ifstream file(mInputFilePath);
  if (!file.is_open()) {
    // Error couldn't open the file
    return false;
  }

  std::string line;
  while (getline(file, line)) {
    mParseLineBehavior(line, showTree);
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

void evaluate(std::string &line, bool showTree) {
  auto globalScope = SymbolTableMgr::getGlobalScope();
  auto syntaxTree = SyntaxTree::Parse(line);
  globalScope->GetTextSpan()->updateTextSpan(0, line.size());
  auto binder = std::make_unique<Binder>();
  auto boundExpression =
      binder->BindExpression(syntaxTree->Root()->Expression());

  if (showTree) {
    syntaxTree->PrintTree();
  }

  if (syntaxTree->Errors().empty() && binder->Errors().empty()) {
    auto eval = std::make_unique<Evaluator>(std::move(boundExpression));
    Value result = eval->Evaluate();
    std::cout << result << std::endl;
  } else {
    for (auto error : syntaxTree->Errors()) {
      std::cerr << error << std::endl;
    }
    for (auto error : binder->Errors()) {
      std::cerr << error << std::endl;
    }
  }
}

void consoleRead(bool &showTree) {
#if !defined(_WIN32) && !defined(__wasm) && !defined(DISABLE_LIBEDIT)
  char *buffer = readline(">>> ");

  // Add input history
  if (buffer[0] != '\0') {
    add_history(buffer);
  }

  std::string line(buffer);
  free(buffer);
#else
  std::string line = "";
  std::cout << ">>> ";
  std::getline(std::cin, line);
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
  evaluate(line, showTree);
}

void startRepl(bool showTree) {
  WarfHelper::printVersion();
  while (true) {
    try {
      consoleRead(showTree);

    } catch (std::runtime_error &error) {
      std::cerr << error.what() << std::endl;
      std::cin.get();
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
    evaluate(evalStr, showTree);
    return 0;
  }
  if (isRepl || argc == 1 || (showTree && argc == 2)) {
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