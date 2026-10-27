// Copyright (c) 2026 F. Lotfi All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "Binding/Binder.h"
#include "IR/Ir.h"
#include "IR/IrOptimizer.h"
#if defined(WARF_ENABLE_LLVM_BACKEND)
#include "LlvmBackend.h"
#endif
#include "Symbol/SymbolTableMgr.h"
#include "Syntax/SyntaxTree.h"
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <unordered_set>

static void printUsage() {
  std::cout << "usage: Warfc [-O|--opt] [--emit-ir|--emit-llvm-ir] [-o output] "
               "<file_name>\n";
  std::cout << "       Warfc -h\n";
  std::cout << "       Warfc --help\n";
}

#if defined(WARF_ENABLE_LLVM_BACKEND)
static std::string ShellQuote(const std::string &value) {
  std::string quoted = "'";
  for (char character : value) {
    if (character == '\'') {
      quoted += "'\\''";
    } else {
      quoted += character;
    }
  }
  return quoted + "'";
}
#endif

static std::string DefaultOutputPath(const std::string &inputPath) {
  const size_t slash = inputPath.find_last_of("/\\");
  const size_t dot = inputPath.find_last_of('.');
  if (dot == std::string::npos || (slash != std::string::npos && dot < slash)) {
    return inputPath + ".out";
  }
  return inputPath.substr(0, dot);
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

static bool ReadSourceFileWithImports(const std::string &path,
                                      std::ostringstream &stream,
                                      std::unordered_set<std::string> &visited,
                                      bool isImported) {
  std::filesystem::path fsPath(path);
  std::string key = NormalizeImportKey(fsPath);
  if (visited.find(key) != visited.end()) {
    return true;
  }

  std::ifstream file(path);
  if (!file.is_open()) {
    if (isImported) {
      std::cerr << "Failed to open imported file: " << path << std::endl;
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
      if (!ReadSourceFileWithImports(resolvedPath.string(), stream, visited,
                                     true)) {
        return false;
      }
      continue;
    }
    stream << line << "\n";
  }

  return !file.bad();
}

static bool ReadSourceFile(const std::string &path, std::string &source) {
  std::ostringstream stream;
  std::unordered_set<std::string> visited;
  if (!ReadSourceFileWithImports(path, stream, visited, false)) {
    return false;
  }
  source = stream.str();
  return true;
}

#if defined(WARF_ENABLE_LLVM_BACKEND)
static bool WriteTextFile(const std::string &path, const std::string &text) {
  std::ofstream output(path);
  if (!output.is_open()) {
    return false;
  }
  output << text;
  return !output.bad();
}
#endif

static bool ReportDiagnostics(const SyntaxTree &syntaxTree,
                              const Binder &binder) {
  bool hasErrors = false;
  for (auto error : syntaxTree.Errors()) {
    std::cerr << error << std::endl;
    hasErrors = true;
  }
  for (auto error : binder.Errors()) {
    std::cerr << error << std::endl;
    hasErrors = true;
  }
  return hasErrors;
}

#if defined(WARF_ENABLE_LLVM_BACKEND)
static bool CompileLLVM(const std::string &llvmIr,
                        const std::string &outputPath) {
  auto llvmPath = outputPath + ".ll";
  if (!WriteTextFile(llvmPath, llvmIr)) {
    return false;
  }

  const std::string command = std::string(WARF_CLANG_EXECUTABLE) + " -x ir " +
                              ShellQuote(llvmPath) + " -o " +
                              ShellQuote(outputPath);
  const int result = std::system(command.c_str());
  std::remove(llvmPath.c_str());
  return result == 0;
}
#endif

int main(int argc, char **argv) {
  bool optimize = false;
  bool emitIr = false;
  bool emitLlvmIr = false;
  std::string inputPath;
  std::string outputPath;

  for (int i = 1; i < argc; i++) {
    std::string arg(argv[i]);
    if (arg == "-h" || arg == "--help") {
      printUsage();
      return 0;
    }
    if (arg == "-O" || arg == "--opt") {
      optimize = true;
      continue;
    }
    if (arg == "--emit-ir") {
      emitIr = true;
      continue;
    }
    if (arg == "--emit-llvm-ir" || arg == "--emit-llvm") {
      emitLlvmIr = true;
      continue;
    }
    if (arg == "-o") {
      if (i + 1 >= argc) {
        std::cerr << "Expected output path after -o." << std::endl;
        return -1;
      }
      outputPath = argv[++i];
      continue;
    }
    if (!arg.empty() && arg[0] == '-') {
      std::cerr << "Invalid flag: " << arg << std::endl;
      printUsage();
      return 0;
    }
    inputPath = arg;
  }

  if (inputPath.empty()) {
    printUsage();
    return 0;
  }
  if (!emitIr && !emitLlvmIr && outputPath.empty()) {
    outputPath = DefaultOutputPath(inputPath);
  }

  SymbolTableMgr::init();

  std::string source;
  if (!ReadSourceFile(inputPath, source)) {
    std::cerr << "Failed to read file: " << inputPath << std::endl;
    return -1;
  }

  auto syntaxTree = SyntaxTree::Parse(source);
  auto binder = std::make_unique<Binder>();
  auto boundStatement = binder->BindCompilationUnit(syntaxTree->Root());
  if (ReportDiagnostics(*syntaxTree, *binder)) {
    return -1;
  }

  IrLowerer lowerer;
  auto module = lowerer.Lower(boundStatement.get());
  if (optimize) {
    IrOptimizer::OptimizeInPlace(module);
  }
  if (emitIr) {
    if (outputPath.empty() || outputPath == "-") {
      IrPrinter::Print(module);
    } else {
      std::stringstream out;
      IrPrinter::Print(module, out);
      std::ofstream output(outputPath);
      if (!output.is_open()) {
        std::cerr << "Failed to write IR: " << outputPath << std::endl;
        return -1;
      }
      output << out.str();
    }
    return 0;
  }
#if defined(WARF_ENABLE_LLVM_BACKEND)
  auto llvmIr = LlvmBackend::EmitLLVM(module, !emitLlvmIr);
  if (emitLlvmIr) {
    if (outputPath.empty() || outputPath == "-") {
      std::cout << llvmIr;
    } else if (!WriteTextFile(outputPath, llvmIr)) {
      std::cerr << "Failed to write LLVM IR: " << outputPath << std::endl;
      return -1;
    }
    return 0;
  }
  if (!CompileLLVM(llvmIr, outputPath)) {
    std::cerr << "Failed to compile executable." << std::endl;
    return -1;
  }

  return 0;
#else
  std::cerr << "LLVM backend is not available in this build." << std::endl;
  return -1;
#endif
}
