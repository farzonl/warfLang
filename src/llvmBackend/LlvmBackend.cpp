// Copyright (c) 2026 F. Lotfi All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "LlvmBackend.h"
#include <llvm/Bitcode/BitcodeWriter.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/raw_ostream.h>
#include <stdexcept>
#include <unordered_map>

static std::string CleanName(const std::string &name) {
  return name.rfind("%", 0) == 0 ? name.substr(1) : name;
}

static bool IsComparison(const std::string &op) {
  return op == "eq" || op == "ne" || op == "lt" || op == "le" || op == "gt" ||
         op == "ge";
}

static llvm::CmpInst::Predicate PredicateFor(const std::string &op) {
  if (op == "eq") {
    return llvm::CmpInst::ICMP_EQ;
  }
  if (op == "ne") {
    return llvm::CmpInst::ICMP_NE;
  }
  if (op == "lt") {
    return llvm::CmpInst::ICMP_SLT;
  }
  if (op == "le") {
    return llvm::CmpInst::ICMP_SLE;
  }
  if (op == "gt") {
    return llvm::CmpInst::ICMP_SGT;
  }
  if (op == "ge") {
    return llvm::CmpInst::ICMP_SGE;
  }
  return llvm::CmpInst::ICMP_EQ;
}

static llvm::Value *
LookupValue(const std::string &name,
            const std::unordered_map<std::string, llvm::Value *> &values) {
  auto value = values.find(name);
  if (value != values.end()) {
    return value->second;
  }
  auto cleanValue = values.find("%" + CleanName(name));
  if (cleanValue != values.end()) {
    return cleanValue->second;
  }
  throw std::runtime_error("LLVM backend error: unknown value " + name + ".");
}

static std::unique_ptr<llvm::Module> BuildModule(const IrModule &irModule,
                                                 bool emitExecutableMain,
                                                 llvm::LLVMContext &context) {
  auto module = std::make_unique<llvm::Module>("warf", context);
  module->setSourceFileName("warf");
  llvm::IRBuilder<> builder(context);
  auto intType = llvm::Type::getInt32Ty(context);
  auto functionMap = std::unordered_map<std::string, llvm::Function *>();

  for (const auto &function : irModule.functions) {
    auto parameterTypes =
        std::vector<llvm::Type *>(function.parameters.size(), intType);
    auto functionType = llvm::FunctionType::get(intType, parameterTypes, false);
    auto functionName = emitExecutableMain && function.name == "__main"
                            ? "main"
                            : function.name;
    auto llvmFunction = llvm::Function::Create(
        functionType, llvm::Function::ExternalLinkage, functionName, *module);
    functionMap[function.name] = llvmFunction;
  }

  for (const auto &function : irModule.functions) {
    auto llvmFunction = functionMap.at(function.name);
    auto values = std::unordered_map<std::string, llvm::Value *>();
    auto parameter = llvmFunction->arg_begin();
    for (const auto &parameterName : function.parameters) {
      parameter->setName(CleanName(parameterName));
      values[parameterName] = &*parameter;
      ++parameter;
    }

    auto block = llvm::BasicBlock::Create(context, "entry", llvmFunction);
    builder.SetInsertPoint(block);

    for (const auto &instruction : function.instructions) {
      llvm::Value *value = nullptr;
      auto resultName = CleanName(instruction.result);
      if (instruction.op == "const") {
        if (instruction.operands[0] == "true") {
          value = llvm::ConstantInt::get(intType, 1);
        } else if (instruction.operands[0] == "false") {
          value = llvm::ConstantInt::get(intType, 0);
        } else {
          value = llvm::ConstantInt::get(intType,
                                         std::stoi(instruction.operands[0]));
        }
      } else if (instruction.op == "copy" || instruction.op == "store") {
        value = LookupValue(instruction.operands[0], values);
      } else if (instruction.op == "neg") {
        value = builder.CreateNeg(LookupValue(instruction.operands[0], values),
                                  resultName);
      } else if (instruction.op == "not") {
        auto compare =
            builder.CreateICmpEQ(LookupValue(instruction.operands[0], values),
                                 llvm::ConstantInt::get(intType, 0));
        value = builder.CreateZExt(compare, intType, resultName);
      } else if (instruction.op == "bnot") {
        value = builder.CreateNot(LookupValue(instruction.operands[0], values),
                                  resultName);
      } else if (instruction.op == "add") {
        value = builder.CreateAdd(LookupValue(instruction.operands[0], values),
                                  LookupValue(instruction.operands[1], values),
                                  resultName);
      } else if (instruction.op == "sub") {
        value = builder.CreateSub(LookupValue(instruction.operands[0], values),
                                  LookupValue(instruction.operands[1], values),
                                  resultName);
      } else if (instruction.op == "mul") {
        value = builder.CreateMul(LookupValue(instruction.operands[0], values),
                                  LookupValue(instruction.operands[1], values),
                                  resultName);
      } else if (instruction.op == "div") {
        value = builder.CreateSDiv(LookupValue(instruction.operands[0], values),
                                   LookupValue(instruction.operands[1], values),
                                   resultName);
      } else if (instruction.op == "band" || instruction.op == "and") {
        value = builder.CreateAnd(LookupValue(instruction.operands[0], values),
                                  LookupValue(instruction.operands[1], values),
                                  resultName);
      } else if (instruction.op == "bor" || instruction.op == "or") {
        value = builder.CreateOr(LookupValue(instruction.operands[0], values),
                                 LookupValue(instruction.operands[1], values),
                                 resultName);
      } else if (instruction.op == "xor") {
        value = builder.CreateXor(LookupValue(instruction.operands[0], values),
                                  LookupValue(instruction.operands[1], values),
                                  resultName);
      } else if (IsComparison(instruction.op)) {
        auto compare =
            builder.CreateICmp(PredicateFor(instruction.op),
                               LookupValue(instruction.operands[0], values),
                               LookupValue(instruction.operands[1], values));
        value = builder.CreateZExt(compare, intType, resultName);
      } else if (instruction.op == "call") {
        auto target = functionMap.find(instruction.operands[0]);
        if (target == functionMap.end()) {
          throw std::runtime_error("LLVM backend error: unknown function " +
                                   instruction.operands[0] + ".");
        }
        auto arguments = std::vector<llvm::Value *>();
        for (size_t i = 1; i < instruction.operands.size(); i++) {
          arguments.push_back(LookupValue(instruction.operands[i], values));
        }
        value = builder.CreateCall(target->second, arguments, resultName);
      } else {
        throw std::runtime_error(
            "LLVM backend error: unsupported instruction " + instruction.op +
            ".");
      }

      if (!instruction.result.empty()) {
        values[instruction.result] = value;
      }
    }

    if (function.returnValue == "void") {
      builder.CreateRet(llvm::ConstantInt::get(intType, 0));
    } else {
      builder.CreateRet(LookupValue(function.returnValue, values));
    }
  }

  std::string verifyError;
  llvm::raw_string_ostream verifyStream(verifyError);
  if (llvm::verifyModule(*module, &verifyStream)) {
    throw std::runtime_error("LLVM backend error: " + verifyStream.str());
  }

  return module;
}

std::string LlvmBackend::EmitLLVM(const IrModule &module,
                                  bool emitExecutableMain) {
  llvm::LLVMContext context;
  auto llvmModule = BuildModule(module, emitExecutableMain, context);
  std::string output;
  llvm::raw_string_ostream stream(output);
  llvmModule->print(stream, nullptr);
  return stream.str();
}

bool LlvmBackend::WriteBitcode(const IrModule &module, const std::string &path,
                               bool emitExecutableMain) {
  llvm::LLVMContext context;
  auto llvmModule = BuildModule(module, emitExecutableMain, context);
  std::error_code error;
  llvm::raw_fd_ostream output(path, error);
  if (error) {
    return false;
  }
  llvm::WriteBitcodeToFile(*llvmModule, output);
  return !output.has_error();
}
