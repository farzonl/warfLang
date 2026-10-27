// Copyright (c) 2026 F. Lotfi All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "IR/Ir.h"
#include "IR/IrOptimizer.h"
#if defined(WARF_ENABLE_LLVM_BACKEND)
#include "LlvmBackend.h"
#endif
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

static void printUsage() {
  std::cout << "usage: ./WarfIR [--opt|-O] --run <ir_file_name>\n";
  std::cout
      << "       ./WarfIR [--opt|-O] [--emit-ir] [-o output] <ir_file_name>\n";
  std::cout << "       ./WarfIR [--opt|-O] --emit-llvm-ir [-o output] "
               "<ir_file_name>\n";
  std::cout
      << "       ./WarfIR [--opt|-O] --emit-bc -o output <ir_file_name>\n";
  std::cout << "       ./WarfIR -h\n";
  std::cout << "       ./WarfIR --help\n";
}

static bool ReadFile(const std::string &path, std::string &text) {
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
    stream << line << std::endl;
  }
  text = stream.str();
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
    if (arg == "--emit-llvm-ir") {
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

  std::string text;
  if (!ReadFile(filePath, text)) {
    std::cerr << "Failed to read file: " << filePath << std::endl;
    return -1;
  }

  IrParser parser;
  auto module = parser.Parse(text);
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
    IrInterpreter interpreter;
    Value result = interpreter.Execute(module);
    if (result.VType() != Value::Type::Unknown) {
      std::cout << result << std::endl;
    }
    return 0;
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
