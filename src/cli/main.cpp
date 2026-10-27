// Copyright (c) 2022 F. Lotfi All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "Binding/Binder.h"
#include "CodeAnalysis/Evaluator.h"
#include "ExpressionStatementSyntaxNode.h"
#include "IR/Ir.h"
#include "Symbol/SymbolTableMgr.h"
#include "Syntax/SyntaxTree.h"
#include "Version/version.h"
#include <cctype>
#include <filesystem>
#include <fstream>
#include <functional>
#include <sstream>
#include <unordered_set>
#include <vector>

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
  static const FlagName emitIr;
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
  bool inString = false;
  for (size_t i = 0; i < line.size(); i++) {
    if (line[i] == '\\' && inString && i + 1 < line.size()) {
      i++;
      continue;
    }
    if (line[i] == '"') {
      inString = !inString;
      continue;
    }
    if (!inString) {
      if (line[i] == '/' && i + 1 < line.size() && line[i + 1] == '/') {
        break;
      }
      if (line[i] == '{') {
        delta++;
      } else if (line[i] == '}') {
        delta--;
      }
    }
  }
  return delta;
}

static std::string StripCommentAndTrim(const std::string &line) {
  std::string text = line;
  bool inString = false;
  size_t commentPos = std::string::npos;
  for (size_t i = 0; i < text.size(); i++) {
    if (text[i] == '\\' && inString && i + 1 < text.size()) {
      i++;
      continue;
    }
    if (text[i] == '"') {
      inString = !inString;
      continue;
    }
    if (!inString && text[i] == '/' && i + 1 < text.size() &&
        text[i + 1] == '/') {
      commentPos = i;
      break;
    }
  }
  if (commentPos != std::string::npos) {
    text = text.substr(0, commentPos);
  }
  size_t start = 0;
  while (start < text.size() &&
         std::isspace(static_cast<unsigned char>(text[start]))) {
    start++;
  }
  size_t end = text.size();
  while (end > start &&
         std::isspace(static_cast<unsigned char>(text[end - 1]))) {
    end--;
  }
  return text.substr(start, end - start);
}

static bool StartsWithKeyword(const std::string &text,
                              const std::string &keyword) {
  if (text.size() < keyword.size() ||
      text.compare(0, keyword.size(), keyword) != 0) {
    return false;
  }
  if (text.size() == keyword.size()) {
    return true;
  }
  unsigned char nextChar = static_cast<unsigned char>(text[keyword.size()]);
  return !std::isalnum(nextChar) && nextChar != '_';
}

static bool StatementStartsWithKeyword(const std::string &statement,
                                       const std::string &keyword) {
  std::stringstream stream(statement);
  std::string line;
  while (std::getline(stream, line)) {
    std::string trimmed = StripCommentAndTrim(line);
    if (!trimmed.empty()) {
      return StartsWithKeyword(trimmed, keyword);
    }
  }
  return false;
}

static bool TryParseImportPath(const std::string &line,
                               std::string &importPath) {
  std::string trimmed = StripCommentAndTrim(line);
  while (!trimmed.empty() &&
         (trimmed.back() == ';' ||
          std::isspace(static_cast<unsigned char>(trimmed.back())))) {
    trimmed.pop_back();
  }
  if (!StartsWithKeyword(trimmed, "import")) {
    return false;
  }
  size_t pos = 6;
  while (pos < trimmed.size() &&
         std::isspace(static_cast<unsigned char>(trimmed[pos]))) {
    pos++;
  }
  if (pos + 2 > trimmed.size() || trimmed[pos] != '"' ||
      trimmed.back() != '"') {
    return false;
  }
  std::string rawPath = trimmed.substr(pos + 1, trimmed.size() - pos - 2);
  if (rawPath.empty() || rawPath.find('"') != std::string::npos) {
    return false;
  }
  importPath = rawPath;
  return true;
}

static std::string NormalizeImportKey(const std::filesystem::path &path) {
  std::error_code ec;
  auto absPath = std::filesystem::absolute(path, ec);
  if (ec) {
    return path.lexically_normal().string();
  }
  auto canonicalPath = std::filesystem::weakly_canonical(absPath, ec);
  if (ec) {
    return absPath.lexically_normal().string();
  }
  return canonicalPath.string();
}

static bool LoadFileLines(const std::string &filePath,
                          std::vector<std::string> &outLines,
                          std::unordered_set<std::string> &visited,
                          bool isImported) {
  std::filesystem::path fsPath(filePath);
  std::string key = NormalizeImportKey(fsPath);
  if (visited.find(key) != visited.end()) {
    return true;
  }

  std::ifstream file(filePath);
  if (!file.is_open()) {
    if (isImported) {
      std::cerr << "Failed to open imported file: " << filePath << std::endl;
    }
    return false;
  }

  visited.insert(key);
  std::filesystem::path baseDir = fsPath.parent_path();
  std::string line;
  while (std::getline(file, line)) {
    std::string importedRelPath;
    if (TryParseImportPath(line, importedRelPath)) {
      std::filesystem::path resolvedPath =
          baseDir.empty() ? std::filesystem::path(importedRelPath)
                          : (baseDir / importedRelPath);
      if (!LoadFileLines(resolvedPath.string(), outLines, visited, true)) {
        return false;
      }
      continue;
    }
    outLines.push_back(line);
  }

  return !file.bad();
}

bool ParseFile::parse(bool showTree) {
  std::vector<std::string> lines;
  std::unordered_set<std::string> visited;
  if (!LoadFileLines(mInputFilePath, lines, visited, false)) {
    return false;
  }

  std::stringstream textBlock;
  std::string statement;
  int32_t braceDepth = 0;
  for (size_t i = 0; i < lines.size(); i++) {
    braceDepth += CountUnmatchedBraces(lines[i]);
    if (!statement.empty()) {
      statement += "\n";
    }
    statement += lines[i];
    if (braceDepth <= 0) {
      std::string nextTrimmed;
      for (size_t j = i + 1; j < lines.size(); j++) {
        nextTrimmed = StripCommentAndTrim(lines[j]);
        if (!nextTrimmed.empty()) {
          break;
        }
      }
      bool continuesWithElse = StartsWithKeyword(nextTrimmed, "else");
      std::string currentTrimmed = StripCommentAndTrim(lines[i]);
      bool continuesWithWhile = StartsWithKeyword(nextTrimmed, "while") &&
                                StatementStartsWithKeyword(statement, "do") &&
                                !currentTrimmed.empty() &&
                                currentTrimmed.back() == '}';
      if (continuesWithElse || continuesWithWhile) {
        braceDepth = 0;
        continue;
      }
      mParseLineBehavior(statement, showTree, textBlock);
      statement.clear();
      braceDepth = 0;
    }
  }
  if (!statement.empty()) {
    mParseLineBehavior(statement, showTree, textBlock);
  }

  return true;
}

const Flags::FlagName Flags::help = {"-h", "--help"};
const Flags::FlagName Flags::eval = {"-e", "--eval"};
const Flags::FlagName Flags::repl = {"-r", "--repl"};
const Flags::FlagName Flags::showTree = {"-s", "--show_tree"};
const Flags::FlagName Flags::emitIr = {"", "--emit-ir"};

void printUsage() {
  std::cout << "usage: ./Warf \n";
  std::cout << "       ./Warf <file_name>\n";
  std::cout << "       ./Warf <file_name> --show_tree\n";
  std::cout << "       ./Warf <file_name> --emit-ir\n";
  std::cout << "       ./Warf --repl\n";
  std::cout << "       ./Warf --eval <string_to_evaluate>\n";
  std::cout << "       ./Warf --eval <string_to_evaluate> --show_tree\n";
  std::cout << "       ./Warf --eval <string_to_evaluate> --emit-ir\n";
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

void evaluate(std::string &line, bool showTree, bool emitIr,
              std::stringstream &textBlock) {
  auto globalScope = SymbolTableMgr::getGlobalScope();
  std::string source = line;
  auto syntaxTree = SyntaxTree::Parse(source);

  // Comment-only/blank lines parse to zero statements; nothing to bind, show,
  // or evaluate.
  if (syntaxTree->Root()->Statements().empty()) {
    return;
  }
  if (!emitIr && syntaxTree->Errors().empty() &&
      syntaxTree->Root()->Statements().size() == 1 &&
      syntaxTree->Root()->Statement()->Kind() ==
          SyntaxKind::FunctionDeclaration) {
    auto testBinder = std::make_unique<Binder>();
    testBinder->BindCompilationUnit(syntaxTree->Root());
    if (testBinder->Errors().empty()) {
      textBlock << line << "\n";
      return;
    }
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
    if (emitIr) {
      IrLowerer lowerer;
      auto module = lowerer.Lower(boundStatement.get());
      IrPrinter::Print(module);
      return;
    }
    try {
      auto eval = std::make_unique<Evaluator>(std::move(boundStatement));
      Value result = eval->Evaluate();
      if (result.VType() != Value::Type::Unknown) {
        std::cout << result << std::endl;
      }
    } catch (std::runtime_error &error) {
      std::cerr << error.what() << std::endl;
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
  evaluate(line, showTree, false, textBlock);
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
  bool emitIr = false;
  int showTreeIndex = 0;
  int emitIrIndex = 0;
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
      i++;
    } else if (Flags::showTree.shortName == argv[i] ||
               Flags::showTree.name == argv[i]) {
      showTree = true;
      showTreeIndex = i;
    } else if (Flags::emitIr.name == argv[i]) {
      emitIr = true;
      emitIrIndex = i;
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
    evaluate(evalStr, showTree, emitIr, textBlock);
    return 0;
  }
  if (isRepl || argc == 1 || ((showTree || emitIr) && argc == 2)) {
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

  int filePathIndex = (1 == showTreeIndex || 1 == emitIrIndex) ? 2 : 1;
  std::string filePath(argv[filePathIndex]);
  if (emitIr) {
    std::vector<std::string> lines;
    std::unordered_set<std::string> visited;
    if (!LoadFileLines(filePath, lines, visited, false)) {
      return -1;
    }
    std::stringstream buffer;
    for (size_t i = 0; i < lines.size(); i++) {
      if (i > 0) {
        buffer << "\n";
      }
      buffer << lines[i];
    }
    std::string wholeFile = buffer.str();
    std::stringstream textBlock;
    evaluate(wholeFile, showTree, emitIr, textBlock);
    return 0;
  }
  ParseFile file(filePath, [emitIr](std::string &line, bool showTree,
                                    std::stringstream &textBlock) {
    evaluate(line, showTree, emitIr, textBlock);
  });
  if (!file.parse(showTree)) {
    return -1;
  }
  return 0;
}
