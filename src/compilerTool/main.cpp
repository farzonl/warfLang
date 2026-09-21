// Copyright (c) 2026 F. Lotfi All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "Binding/Binder.h"
#include "IR/Ir.h"
#if defined(WARF_ENABLE_LLVM_BACKEND)
#include "LlvmBackend.h"
#endif
#include "Symbol/SymbolTableMgr.h"
#include "Syntax/SyntaxTree.h"
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

static void printUsage() {
  std::cout << "usage: warfc [-o output] <file_name>\n";
  std::cout << "       warfc -h\n";
  std::cout << "       warfc --help\n";
}

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

static std::string DefaultOutputPath(const std::string &inputPath) {
  const size_t slash = inputPath.find_last_of("/\\");
  const size_t dot = inputPath.find_last_of('.');
  if (dot == std::string::npos || (slash != std::string::npos && dot < slash)) {
    return inputPath + ".out";
  }
  return inputPath.substr(0, dot);
}

static bool ReadSourceFile(const std::string &path, std::string &source) {
  std::ifstream file(path);
  if (!file.is_open()) {
    return false;
  }

  std::stringstream stream;
  std::string line;
  while (std::getline(file, line)) {
    auto comment = line.find("//");
    if (comment != std::string::npos) {
      line = line.substr(0, comment);
    }
    stream << line << " ";
  }
  source = stream.str();
  return !file.bad();
}

static bool WriteTextFile(const std::string &path, const std::string &text) {
  std::ofstream output(path);
  if (!output.is_open()) {
    return false;
  }
  output << text;
  return !output.bad();
}

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

int main(int argc, char **argv) {
  std::string inputPath;
  std::string outputPath;

  for (int i = 1; i < argc; i++) {
    std::string arg(argv[i]);
    if (arg == "-h" || arg == "--help") {
      printUsage();
      return 0;
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
  if (outputPath.empty()) {
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
#if defined(WARF_ENABLE_LLVM_BACKEND)
  auto llvmIr = LlvmBackend::EmitLLVM(module, true);
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