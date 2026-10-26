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
#include <unordered_set>

static std::string CleanName(const std::string &name) {
  return name.rfind("%", 0) == 0 ? name.substr(1) : name;
}

static std::string CanonicalName(const std::string &name) {
  return "%" + CleanName(name);
}

static llvm::CmpInst::Predicate PredicateFor(IrOpcode op) {
  switch (op) {
  case IrOpcode::Eq:
    return llvm::CmpInst::ICMP_EQ;
  case IrOpcode::Ne:
    return llvm::CmpInst::ICMP_NE;
  case IrOpcode::Lt:
    return llvm::CmpInst::ICMP_SLT;
  case IrOpcode::Le:
    return llvm::CmpInst::ICMP_SLE;
  case IrOpcode::Gt:
    return llvm::CmpInst::ICMP_SGT;
  case IrOpcode::Ge:
    return llvm::CmpInst::ICMP_SGE;
  default:
    return llvm::CmpInst::ICMP_EQ;
  }
}

static llvm::BasicBlock *
LookupBlock(const std::string &name,
            const std::unordered_map<std::string, llvm::BasicBlock *> &blocks) {
  auto block = blocks.find(name);
  if (block != blocks.end()) {
    return block->second;
  }
  throw std::runtime_error("LLVM backend error: unknown label " + name + ".");
}

static llvm::Value *
LookupValue(const std::string &name,
            const std::unordered_map<std::string, llvm::Value *> &values,
            const std::unordered_map<std::string, llvm::AllocaInst *> &slots,
            llvm::Type *intType, llvm::IRBuilder<> &builder) {
  auto canonical = CanonicalName(name);
  auto slot = slots.find(canonical);
  if (slot != slots.end()) {
    return builder.CreateLoad(intType, slot->second, CleanName(name));
  }
  auto value = values.find(name);
  if (value != values.end()) {
    return value->second;
  }
  auto cleanValue = values.find(canonical);
  if (cleanValue != values.end()) {
    return cleanValue->second;
  }
  throw std::runtime_error("LLVM backend error: unknown value " + name + ".");
}

static void EmitBranchIfUnterminated(llvm::IRBuilder<> &builder,
                                     llvm::BasicBlock *targetBlock) {
  if (!builder.GetInsertBlock()->getTerminator()) {
    builder.CreateBr(targetBlock);
  }
}

static llvm::Value *EmitUnaryOp(IrOpcode op, llvm::Value *operand,
                                const std::string &resultName,
                                llvm::Type *intType,
                                llvm::IRBuilder<> &builder) {
  switch (op) {
  case IrOpcode::Copy:
    return operand;
  case IrOpcode::Neg:
    return builder.CreateNeg(operand, resultName);
  case IrOpcode::Not: {
    auto compare =
        builder.CreateICmpEQ(operand, llvm::ConstantInt::get(intType, 0));
    return builder.CreateZExt(compare, intType, resultName);
  }
  case IrOpcode::BitwiseNot:
    return builder.CreateNot(operand, resultName);
  default:
    throw std::runtime_error("LLVM backend error: unsupported instruction " +
                             IrOpcodeToString(op) + ".");
  }
}

static llvm::Value *EmitBinaryOp(IrOpcode op, llvm::Value *left,
                                 llvm::Value *right,
                                 const std::string &resultName,
                                 llvm::Type *intType,
                                 llvm::IRBuilder<> &builder) {
  if (IsComparisonOp(op)) {
    auto compare = builder.CreateICmp(PredicateFor(op), left, right);
    return builder.CreateZExt(compare, intType, resultName);
  }
  switch (op) {
  case IrOpcode::Add:
    return builder.CreateAdd(left, right, resultName);
  case IrOpcode::Sub:
    return builder.CreateSub(left, right, resultName);
  case IrOpcode::Mul:
    return builder.CreateMul(left, right, resultName);
  case IrOpcode::Div:
    return builder.CreateSDiv(left, right, resultName);
  case IrOpcode::BitwiseAnd:
  case IrOpcode::And:
    return builder.CreateAnd(left, right, resultName);
  case IrOpcode::BitwiseOr:
  case IrOpcode::Or:
    return builder.CreateOr(left, right, resultName);
  case IrOpcode::Xor:
    return builder.CreateXor(left, right, resultName);
  default:
    throw std::runtime_error("LLVM backend error: unsupported instruction " +
                             IrOpcodeToString(op) + ".");
  }
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
    auto functionName =
        emitExecutableMain && function.name == kIrMainFunctionName
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
      values[CanonicalName(parameterName)] = &*parameter;
      ++parameter;
    }

    const auto &entryLabel = IrLabelPrefix(IrLabelKind::Entry);
    auto blocks = std::unordered_map<std::string, llvm::BasicBlock *>();
    auto entryBlock =
        llvm::BasicBlock::Create(context, entryLabel, llvmFunction);
    blocks[entryLabel] = entryBlock;

    auto slotVarOrder = std::vector<std::string>();
    auto slotVarSet = std::unordered_set<std::string>();
    auto defCounts = std::unordered_map<std::string, size_t>();
    auto addSlotVar = [&](const std::string &varName) {
      auto canonical = CanonicalName(varName);
      if (slotVarSet.insert(canonical).second) {
        slotVarOrder.push_back(canonical);
      }
    };

    for (const auto &instruction : function.instructions) {
      if (instruction.op == IrOpcode::Label && !instruction.operands.empty()) {
        const auto &labelName = instruction.operands[0];
        if (blocks.find(labelName) == blocks.end()) {
          blocks[labelName] =
              llvm::BasicBlock::Create(context, labelName, llvmFunction);
        }
        continue;
      }
      if (!instruction.result.empty()) {
        auto canonical = CanonicalName(instruction.result);
        defCounts[canonical]++;
        if (IsStoreOp(instruction.op) || defCounts[canonical] > 1) {
          addSlotVar(canonical);
        }
      }
    }

    builder.SetInsertPoint(entryBlock);
    auto slots = std::unordered_map<std::string, llvm::AllocaInst *>();
    if (blocks.size() > 1) {
      for (const auto &varName : slotVarOrder) {
        auto allocaInst = builder.CreateAlloca(intType, nullptr,
                                               CleanName(varName) + ".addr");
        auto paramValue = values.find(varName);
        llvm::Value *initialValue = paramValue != values.end()
                                        ? paramValue->second
                                        : llvm::ConstantInt::get(intType, 0);
        builder.CreateStore(initialValue, allocaInst);
        slots[varName] = allocaInst;
      }
    }

    for (const auto &instruction : function.instructions) {
      auto operand = [&](size_t index) {
        return LookupValue(instruction.operands[index], values, slots, intType,
                           builder);
      };

      if (instruction.op == IrOpcode::Label) {
        auto nextBlock = LookupBlock(instruction.operands[0], blocks);
        EmitBranchIfUnterminated(builder, nextBlock);
        builder.SetInsertPoint(nextBlock);
        continue;
      }
      if (instruction.op == IrOpcode::Br) {
        EmitBranchIfUnterminated(builder,
                                 LookupBlock(instruction.operands[0], blocks));
        continue;
      }
      if (instruction.op == IrOpcode::BrCond) {
        auto condValue = operand(0);
        auto trueBlock = LookupBlock(instruction.operands[1], blocks);
        auto falseBlock = LookupBlock(instruction.operands[2], blocks);
        if (!builder.GetInsertBlock()->getTerminator()) {
          auto condBool = builder.CreateICmpNE(
              condValue, llvm::ConstantInt::get(intType, 0), "cond");
          builder.CreateCondBr(condBool, trueBlock, falseBlock);
        }
        continue;
      }

      llvm::Value *value = nullptr;
      auto resultName = CleanName(instruction.result);
      if (instruction.op == IrOpcode::Const) {
        if (instruction.operands[0] == kIrTrueLiteral) {
          value = llvm::ConstantInt::get(intType, 1);
        } else if (instruction.operands[0] == kIrFalseLiteral) {
          value = llvm::ConstantInt::get(intType, 0);
        } else {
          value = llvm::ConstantInt::get(intType,
                                         std::stoi(instruction.operands[0]));
        }
      } else if (instruction.op == IrOpcode::Store) {
        value = operand(0);
      } else if (IsUnaryOp(instruction.op)) {
        value = EmitUnaryOp(instruction.op, operand(0), resultName, intType,
                            builder);
      } else if (IsBinaryOp(instruction.op)) {
        value = EmitBinaryOp(instruction.op, operand(0), operand(1), resultName,
                             intType, builder);
      } else if (IsCompoundStoreOp(instruction.op)) {
        auto current =
            LookupValue(instruction.result, values, slots, intType, builder);
        value = EmitBinaryOp(CompoundStoreToBinaryOp(instruction.op), current,
                             operand(0), resultName, intType, builder);
      } else if (instruction.op == IrOpcode::Call) {
        auto target = functionMap.find(instruction.operands[0]);
        if (target == functionMap.end()) {
          throw std::runtime_error("LLVM backend error: unknown function " +
                                   instruction.operands[0] + ".");
        }
        auto arguments = std::vector<llvm::Value *>();
        for (size_t i = 1; i < instruction.operands.size(); i++) {
          arguments.push_back(operand(i));
        }
        value = builder.CreateCall(target->second, arguments, resultName);
      } else {
        throw std::runtime_error(
            "LLVM backend error: unsupported instruction " +
            IrOpcodeToString(instruction.op) + ".");
      }

      if (!instruction.result.empty()) {
        auto canonical = CanonicalName(instruction.result);
        auto slot = slots.find(canonical);
        if (slot != slots.end()) {
          builder.CreateStore(value, slot->second);
        } else {
          values[instruction.result] = value;
          values[canonical] = value;
        }
      }
    }

    if (!builder.GetInsertBlock()->getTerminator()) {
      if (function.returnValue == kIrVoidValue) {
        builder.CreateRet(llvm::ConstantInt::get(intType, 0));
      } else {
        builder.CreateRet(
            LookupValue(function.returnValue, values, slots, intType, builder));
      }
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
