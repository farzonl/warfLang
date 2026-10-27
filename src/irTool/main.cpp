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
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_set>

static void printUsage() {
  std::cout << "usage: ./WarfIR [--opt|-O] --run <ir_or_wf_file_name>\n";
  std::cout << "       ./WarfIR [--opt|-O] [--emit-ir] [-o output] "
               "<ir_or_wf_file_name>\n";
  std::cout << "       ./WarfIR [--opt|-O] --emit-llvm-ir [-o output] "
               "<ir_or_wf_file_name>\n";
  std::cout << "       ./WarfIR [--opt|-O] --emit-bc -o output "
               "<ir_or_wf_file_name>\n";
  std::cout << "       ./WarfIR -h\n";
  std::cout << "       ./WarfIR --help\n";
}

static bool HasWfExtension(const std::string &path) {
  return path.size() >= 3 && path.compare(path.size() - 3, 3, ".wf") == 0;
}

static std::string StripComment(const std::string &line) {
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
    if (!inString && line[i] == '/' && i + 1 < line.size() &&
        line[i + 1] == '/') {
      return line.substr(0, i);
    }
  }
  return line;
}

static std::string StripCommentAndTrim(const std::string &line) {
  std::string text = StripComment(line);
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

static bool LowerSourceToModule(const std::string &source, IrModule &module) {
  SymbolTableMgr::init();
  auto syntaxTree = SyntaxTree::Parse(source);
  auto binder = std::make_unique<Binder>();
  auto boundStatement = binder->BindCompilationUnit(syntaxTree->Root());
  bool hasErrors = false;
  for (const auto &error : syntaxTree->Errors()) {
    std::cerr << error << std::endl;
    hasErrors = true;
  }
  for (const auto &error : binder->Errors()) {
    std::cerr << error << std::endl;
    hasErrors = true;
  }
  if (hasErrors) {
    return false;
  }
  IrLowerer lowerer;
  module = lowerer.Lower(boundStatement.get());
  return true;
}

static bool ReadFileWithImports(const std::string &path,
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
      if (!ReadFileWithImports(resolvedPath.string(), stream, visited, true)) {
        return false;
      }
      continue;
    }
    stream << StripComment(line) << std::endl;
  }

  return !file.bad();
}

static bool ReadFile(const std::string &path, std::string &text) {
  std::ostringstream stream;
  std::unordered_set<std::string> visited;
  if (!ReadFileWithImports(path, stream, visited, false)) {
    return false;
  }
  text = stream.str();
  return true;
}

static bool WriteTextFile(const std::string &path, const std::string &text) {
  std::ofstream output(path);
  if (!output.is_open()) {
    return false;
  }
  output << text;
  return !output.bad();
}

int main(int argc, char **argv) {
  bool run = false;
  bool emitIr = false;
  bool emitLlvmIr = false;
  bool emitBitcode = false;
  bool optimize = false;
  std::string filePath;
  std::string outputPath;

  for (int i = 1; i < argc; i++) {
    std::string arg(argv[i]);
    if (arg == "-h" || arg == "--help") {
      printUsage();
      return 0;
    }
    if (arg == "--run") {
      run = true;
      continue;
    }
    if (arg == "--emit-ir") {
      emitIr = true;
      continue;
    }
    if (arg == "--opt" || arg == "-O") {
      optimize = true;
      continue;
    }
    if (arg == "--emit-llvm-ir" || arg == "--emit-llvm") {
      emitLlvmIr = true;
      continue;
    }
    if (arg == "--emit-bc") {
      emitBitcode = true;
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
    filePath = arg;
  }

  if (filePath.empty()) {
    printUsage();
    return 0;
  }
  if (optimize && !run && !emitIr && !emitLlvmIr && !emitBitcode) {
    emitIr = true;
  }
  if ((run ? 1 : 0) + (emitIr ? 1 : 0) + (emitLlvmIr ? 1 : 0) +
          (emitBitcode ? 1 : 0) !=
      1) {
    std::cerr << "Expected exactly one action: --run, --emit-ir, "
                 "--emit-llvm-ir, or --emit-bc."
              << std::endl;
    printUsage();
    return -1;
  }
  if (emitBitcode && outputPath.empty()) {
    std::cerr << "Expected -o output for --emit-bc." << std::endl;
    return -1;
  }

  IrModule module;
  if (HasWfExtension(filePath)) {
    std::string source;
    if (!ReadSourceFile(filePath, source)) {
      std::cerr << "Failed to read file: " << filePath << std::endl;
      return -1;
    }
    if (!LowerSourceToModule(source, module)) {
      return -1;
    }
  } else {
    std::string text;
    if (!ReadFile(filePath, text)) {
      std::cerr << "Failed to read file: " << filePath << std::endl;
      return -1;
    }
    IrParser parser;
    module = parser.Parse(text);
  }
  if (optimize) {
    IrOptimizer::OptimizeInPlace(module);
  }

  if (emitIr) {
    if (outputPath.empty() || outputPath == "-") {
      IrPrinter::Print(module);
    } else {
      std::stringstream out;
      IrPrinter::Print(module, out);
      if (!WriteTextFile(outputPath, out.str())) {
        std::cerr << "Failed to write IR: " << outputPath << std::endl;
        return -1;
      }
    }
    return 0;
  }

  if (run) {
    try {
      IrInterpreter interpreter;
      Value result = interpreter.Execute(module);
      if (result.VType() != Value::Type::Unknown) {
        std::cout << result << std::endl;
      }
      return 0;
    } catch (std::runtime_error &error) {
      std::cerr << error.what() << std::endl;
      return -1;
    }
  }

  if (emitLlvmIr) {
#if defined(WARF_ENABLE_LLVM_BACKEND)
    auto llvmIr = LlvmBackend::EmitLLVM(module);
    if (outputPath.empty() || outputPath == "-") {
      std::cout << llvmIr;
    } else if (!WriteTextFile(outputPath, llvmIr)) {
      std::cerr << "Failed to write LLVM IR: " << outputPath << std::endl;
      return -1;
    }
    return 0;
#else
    std::cerr << "LLVM backend is not available in this build." << std::endl;
    return -1;
#endif
  }
  if (emitBitcode) {
#if defined(WARF_ENABLE_LLVM_BACKEND)
    if (!LlvmBackend::WriteBitcode(module, outputPath)) {
      std::cerr << "Failed to write LLVM bitcode." << std::endl;
      return -1;
    }
    return 0;
#else
    std::cerr << "LLVM backend is not available in this build." << std::endl;
    return -1;
#endif
  }

  return -1;
}
