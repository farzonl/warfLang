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

static bool IsStringLiteral(const std::string &s) {
  return s.size() >= 2 && s.front() == '"' && s.back() == '"';
}

struct SlotInfo {
  llvm::AllocaInst *allocaInst;
  llvm::Type *type;
};

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
            const std::unordered_map<std::string, SlotInfo> &slots,
            llvm::Type *intType, llvm::IRBuilder<> &builder) {
  if (IsStringLiteral(name)) {
    return builder.CreateGlobalString(UnescapeIrString(name), ".str");
  }
  if (name == kIrTrueLiteral) {
    return llvm::ConstantInt::get(intType, 1);
  }
  if (name == kIrFalseLiteral) {
    return llvm::ConstantInt::get(intType, 0);
  }
  auto canonical = CanonicalName(name);
  auto slot = slots.find(canonical);
  if (slot != slots.end()) {
    return builder.CreateLoad(slot->second.type, slot->second.allocaInst,
                              CleanName(name));
  }
  auto value = values.find(name);
  if (value != values.end()) {
    return value->second;
  }
  auto cleanValue = values.find(canonical);
  if (cleanValue != values.end()) {
    return cleanValue->second;
  }
  if (!name.empty() && (std::isdigit(static_cast<unsigned char>(name[0])) ||
                        (name[0] == '-' && name.size() > 1 &&
                         std::isdigit(static_cast<unsigned char>(name[1]))))) {
    return llvm::ConstantInt::get(intType,
                                  static_cast<int32_t>(std::stoll(name)));
  }
  throw std::runtime_error("LLVM backend error: unknown value " + name + ".");
}

static bool HasTerminator(const llvm::BasicBlock *block) {
  return !block->empty() && block->back().isTerminator();
}

static void EmitBranchIfUnterminated(llvm::IRBuilder<> &builder,
                                     llvm::BasicBlock *targetBlock) {
  if (!HasTerminator(builder.GetInsertBlock())) {
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

static llvm::Value *
EmitBinaryOp(IrOpcode op, llvm::Value *left, llvm::Value *right,
             const std::string &resultName, llvm::Type *intType,
             llvm::IRBuilder<> &builder, llvm::Module *module) {
  auto &context = builder.getContext();
  auto ptrType = llvm::PointerType::getUnqual(context);

  if (left->getType()->isPointerTy() || right->getType()->isPointerTy()) {
    if (op == IrOpcode::Add) {
      auto sizeType = builder.getInt64Ty();
      auto strlenType = llvm::FunctionType::get(sizeType, {ptrType}, false);
      auto strlenFunc = module->getOrInsertFunction("strlen", strlenType);

      auto mallocType = llvm::FunctionType::get(ptrType, {sizeType}, false);
      auto mallocFunc = module->getOrInsertFunction("malloc", mallocType);

      auto strcpyType =
          llvm::FunctionType::get(ptrType, {ptrType, ptrType}, false);
      auto strcpyFunc = module->getOrInsertFunction("strcpy", strcpyType);

      auto strcatType =
          llvm::FunctionType::get(ptrType, {ptrType, ptrType}, false);
      auto strcatFunc = module->getOrInsertFunction("strcat", strcatType);

      auto len1 = builder.CreateCall(strlenFunc, {left}, "len1");
      auto len2 = builder.CreateCall(strlenFunc, {right}, "len2");
      auto sumLen = builder.CreateAdd(len1, len2, "sumlen");
      auto totalLen = builder.CreateAdd(
          sumLen, llvm::ConstantInt::get(sizeType, 1), "totallen");
      auto newBuf = builder.CreateCall(
          mallocFunc, {totalLen}, resultName.empty() ? "strbuf" : resultName);
      builder.CreateCall(strcpyFunc, {newBuf, left});
      builder.CreateCall(strcatFunc, {newBuf, right});
      return newBuf;
    }
    if (op == IrOpcode::Eq || op == IrOpcode::Ne) {
      auto strcmpType =
          llvm::FunctionType::get(intType, {ptrType, ptrType}, false);
      auto strcmpFunc = module->getOrInsertFunction("strcmp", strcmpType);
      auto cmpRes = builder.CreateCall(strcmpFunc, {left, right}, "strcmp.res");
      if (op == IrOpcode::Eq) {
        auto isEqual = builder.CreateICmpEQ(
            cmpRes, llvm::ConstantInt::get(intType, 0), "streq");
        return builder.CreateZExt(isEqual, intType, resultName);
      } else {
        auto isNotEqual = builder.CreateICmpNE(
            cmpRes, llvm::ConstantInt::get(intType, 0), "strne");
        return builder.CreateZExt(isNotEqual, intType, resultName);
      }
    }
    throw std::runtime_error(
        "LLVM backend error: unsupported binary operation on strings " +
        IrOpcodeToString(op) + ".");
  }

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
  case IrOpcode::Mod: {
    auto &context = builder.getContext();
    auto isZero = builder.CreateICmpEQ(
        right, llvm::ConstantInt::get(intType, 0), "is_divzero");

    auto *llvmFunction = builder.GetInsertBlock()->getParent();
    auto *errBlock =
        llvm::BasicBlock::Create(context, "divzero.err", llvmFunction);
    auto *contBlock =
        llvm::BasicBlock::Create(context, "divzero.cont", llvmFunction);

    builder.CreateCondBr(isZero, errBlock, contBlock);

    builder.SetInsertPoint(errBlock);
    auto printfType = llvm::FunctionType::get(
        intType, {llvm::PointerType::getUnqual(context)}, true);
    auto printfFunc = module->getOrInsertFunction("printf", printfType);
    auto msgPtr = builder.CreateGlobalString("Logial error: Divide by zero.\n",
                                             ".divzero.msg");
    builder.CreateCall(printfFunc, {msgPtr});

    auto voidType = llvm::Type::getVoidTy(context);
    auto exitType = llvm::FunctionType::get(voidType, {intType}, false);
    auto exitFunc = module->getOrInsertFunction("exit", exitType);
    builder.CreateCall(exitFunc, {llvm::ConstantInt::get(intType, 1)});
    builder.CreateUnreachable();

    builder.SetInsertPoint(contBlock);
    if (op == IrOpcode::Div) {
      return builder.CreateSDiv(left, right, resultName);
    }
    return builder.CreateSRem(left, right, resultName);
  }
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
  auto ptrType = llvm::PointerType::getUnqual(context);
  auto functionMap = std::unordered_map<std::string, llvm::Function *>();

  // Module-level type inference to identify string variables, parameters, and
  // return types
  std::unordered_map<std::string, std::unordered_set<std::string>>
      functionStringVars;
  std::unordered_map<std::string, bool> functionReturnsString;
  std::unordered_map<std::string, const IrFunction *> functionTable;
  for (const auto &function : irModule.functions) {
    functionTable[function.name] = &function;
  }

  auto isStringOperand = [&](const std::string &fnName,
                             const std::string &operand) -> bool {
    if (IsStringLiteral(operand)) {
      return true;
    }
    auto canonical = CanonicalName(operand);
    auto it = functionStringVars.find(fnName);
    if (it != functionStringVars.end()) {
      return it->second.count(canonical) > 0;
    }
    return false;
  };

  auto markString = [&](const std::string &fnName,
                        const std::string &varName) -> bool {
    if (!varName.empty() && !IsStringLiteral(varName) &&
        varName != kIrVoidValue && varName != kIrTrueLiteral &&
        varName != kIrFalseLiteral) {
      if (!(std::isdigit(static_cast<unsigned char>(varName[0])) ||
            (varName[0] == '-' && varName.size() > 1 &&
             std::isdigit(static_cast<unsigned char>(varName[1]))))) {
        return functionStringVars[fnName].insert(CanonicalName(varName)).second;
      }
    }
    return false;
  };

  auto markReturnsString = [&](const std::string &fnName) -> bool {
    if (!functionReturnsString[fnName]) {
      functionReturnsString[fnName] = true;
      return true;
    }
    return false;
  };

  bool changed = true;
  while (changed) {
    changed = false;
    for (const auto &function : irModule.functions) {
      const auto &fnName = function.name;

      if (!function.returnValue.empty() &&
          function.returnValue != kIrVoidValue) {
        if (isStringOperand(fnName, function.returnValue)) {
          if (markReturnsString(fnName)) {
            changed = true;
          }
        }
      }

      for (const auto &instruction : function.instructions) {
        if (instruction.op == IrOpcode::Const &&
            !instruction.operands.empty()) {
          if (IsStringLiteral(instruction.operands[0])) {
            if (markString(fnName, instruction.result)) {
              changed = true;
            }
          }
        } else if ((instruction.op == IrOpcode::Copy ||
                    instruction.op == IrOpcode::Store) &&
                   !instruction.operands.empty()) {
          if (isStringOperand(fnName, instruction.operands[0])) {
            if (markString(fnName, instruction.result)) {
              changed = true;
            }
          }
          if (isStringOperand(fnName, instruction.result)) {
            if (markString(fnName, instruction.operands[0])) {
              changed = true;
            }
          }
        } else if (instruction.op == IrOpcode::Add &&
                   instruction.operands.size() >= 2) {
          bool op0Str = isStringOperand(fnName, instruction.operands[0]);
          bool op1Str = isStringOperand(fnName, instruction.operands[1]);
          bool resStr = isStringOperand(fnName, instruction.result);
          if (op0Str || op1Str || resStr) {
            if (markString(fnName, instruction.result))
              changed = true;
            if (markString(fnName, instruction.operands[0]))
              changed = true;
            if (markString(fnName, instruction.operands[1]))
              changed = true;
          }
        } else if (instruction.op == IrOpcode::StoreAdd &&
                   !instruction.operands.empty()) {
          bool resStr = isStringOperand(fnName, instruction.result);
          bool op0Str = isStringOperand(fnName, instruction.operands[0]);
          if (resStr || op0Str) {
            if (markString(fnName, instruction.result))
              changed = true;
            if (markString(fnName, instruction.operands[0]))
              changed = true;
          }
        } else if ((instruction.op == IrOpcode::Eq ||
                    instruction.op == IrOpcode::Ne) &&
                   instruction.operands.size() >= 2) {
          bool op0Str = isStringOperand(fnName, instruction.operands[0]);
          bool op1Str = isStringOperand(fnName, instruction.operands[1]);
          if (op0Str && !op1Str) {
            if (markString(fnName, instruction.operands[1]))
              changed = true;
          } else if (op1Str && !op0Str) {
            if (markString(fnName, instruction.operands[0]))
              changed = true;
          }
        } else if (instruction.op == IrOpcode::Print &&
                   !instruction.operands.empty()) {
          if (isStringOperand(fnName, instruction.operands[0])) {
            if (markString(fnName, instruction.result))
              changed = true;
          }
          if (isStringOperand(fnName, instruction.result)) {
            if (markString(fnName, instruction.operands[0]))
              changed = true;
          }
        } else if (instruction.op == IrOpcode::Ret &&
                   !instruction.operands.empty()) {
          if (instruction.operands[0] != kIrVoidValue) {
            if (isStringOperand(fnName, instruction.operands[0])) {
              if (markReturnsString(fnName))
                changed = true;
            }
            if (functionReturnsString[fnName]) {
              if (markString(fnName, instruction.operands[0]))
                changed = true;
            }
          }
        } else if (instruction.op == IrOpcode::Call &&
                   !instruction.operands.empty()) {
          const auto &targetName = instruction.operands[0];
          auto calleeIt = functionTable.find(targetName);
          if (calleeIt != functionTable.end()) {
            const auto *callee = calleeIt->second;
            for (size_t i = 1; i < instruction.operands.size(); ++i) {
              size_t paramIdx = i - 1;
              if (paramIdx < callee->parameters.size()) {
                const auto &arg = instruction.operands[i];
                const auto &param = callee->parameters[paramIdx];
                if (isStringOperand(fnName, arg)) {
                  if (markString(callee->name, param))
                    changed = true;
                }
                if (isStringOperand(callee->name, param)) {
                  if (markString(fnName, arg))
                    changed = true;
                }
              }
            }
            if (functionReturnsString[callee->name]) {
              if (markString(fnName, instruction.result))
                changed = true;
            }
            if (isStringOperand(fnName, instruction.result)) {
              if (markReturnsString(callee->name))
                changed = true;
            }
          }
        }
      }
    }
  }

  for (const auto &function : irModule.functions) {
    auto parameterTypes = std::vector<llvm::Type *>();
    parameterTypes.reserve(function.parameters.size());
    for (const auto &paramName : function.parameters) {
      bool isStr = isStringOperand(function.name, paramName);
      parameterTypes.push_back(isStr ? static_cast<llvm::Type *>(ptrType)
                                     : static_cast<llvm::Type *>(intType));
    }
    llvm::Type *returnType = intType;
    if (function.name != kIrMainFunctionName &&
        functionReturnsString[function.name]) {
      returnType = ptrType;
    }
    auto functionType =
        llvm::FunctionType::get(returnType, parameterTypes, false);
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
    auto defBlock = std::unordered_map<std::string, std::string>();
    auto addSlotVar = [&](const std::string &varName) {
      auto canonical = CanonicalName(varName);
      if (slotVarSet.insert(canonical).second) {
        slotVarOrder.push_back(canonical);
      }
    };

    std::string currentBlockName = entryLabel;
    for (const auto &instruction : function.instructions) {
      if (instruction.op == IrOpcode::Label && !instruction.operands.empty()) {
        const auto &labelName = instruction.operands[0];
        if (blocks.find(labelName) == blocks.end()) {
          blocks[labelName] =
              llvm::BasicBlock::Create(context, labelName, llvmFunction);
        }
        currentBlockName = labelName;
        continue;
      }
      if (!instruction.result.empty()) {
        auto canonical = CanonicalName(instruction.result);
        defCounts[canonical]++;
        defBlock[canonical] = currentBlockName;
        if (IsStoreOp(instruction.op) || defCounts[canonical] > 1) {
          addSlotVar(canonical);
        }
      }
    }

    currentBlockName = entryLabel;
    for (const auto &instruction : function.instructions) {
      if (instruction.op == IrOpcode::Label && !instruction.operands.empty()) {
        currentBlockName = instruction.operands[0];
        continue;
      }
      for (const auto &op : instruction.operands) {
        if (!op.empty() && op[0] == '%') {
          auto canonical = CanonicalName(op);
          auto it = defBlock.find(canonical);
          if (it != defBlock.end() && it->second != entryLabel &&
              it->second != currentBlockName) {
            addSlotVar(canonical);
          }
        }
      }
    }
    if (!function.returnValue.empty() && function.returnValue[0] == '%') {
      auto canonical = CanonicalName(function.returnValue);
      auto it = defBlock.find(canonical);
      if (it != defBlock.end() && it->second != entryLabel &&
          it->second != currentBlockName) {
        addSlotVar(canonical);
      }
    }

    const auto &stringVars = functionStringVars[function.name];

    builder.SetInsertPoint(entryBlock);
    auto slots = std::unordered_map<std::string, SlotInfo>();
    if (blocks.size() > 1) {
      for (const auto &varName : slotVarOrder) {
        bool isStr = stringVars.count(varName) > 0;
        llvm::Type *slotType = isStr ? static_cast<llvm::Type *>(ptrType)
                                     : static_cast<llvm::Type *>(intType);
        auto allocaInst = builder.CreateAlloca(slotType, nullptr,
                                               CleanName(varName) + ".addr");
        auto paramValue = values.find(varName);
        llvm::Value *initialValue = nullptr;
        if (paramValue != values.end()) {
          initialValue = paramValue->second;
        } else if (isStr) {
          initialValue = llvm::ConstantPointerNull::get(ptrType);
        } else {
          initialValue = llvm::ConstantInt::get(intType, 0);
        }
        builder.CreateStore(initialValue, allocaInst);
        slots[varName] = {allocaInst, slotType};
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
      if (HasTerminator(builder.GetInsertBlock())) {
        continue;
      }
      if (instruction.op == IrOpcode::Ret) {
        if (instruction.operands[0] == kIrVoidValue) {
          builder.CreateRet(llvm::ConstantInt::get(intType, 0));
        } else {
          auto retVal = operand(0);
          if (function.name == kIrMainFunctionName &&
              retVal->getType()->isPointerTy()) {
            builder.CreateRet(llvm::ConstantInt::get(intType, 0));
          } else if (llvmFunction->getReturnType() == intType &&
                     retVal->getType()->isPointerTy()) {
            builder.CreateRet(builder.CreatePtrToInt(retVal, intType));
          } else if (llvmFunction->getReturnType() == ptrType &&
                     retVal->getType() == intType) {
            builder.CreateRet(builder.CreateIntToPtr(retVal, ptrType));
          } else {
            builder.CreateRet(retVal);
          }
        }
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
        if (!HasTerminator(builder.GetInsertBlock())) {
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
        } else if (IsStringLiteral(instruction.operands[0])) {
          value = builder.CreateGlobalString(
              UnescapeIrString(instruction.operands[0]), ".str");
        } else {
          value = llvm::ConstantInt::get(
              intType,
              static_cast<int32_t>(std::stoll(instruction.operands[0])));
        }
      } else if (instruction.op == IrOpcode::Store) {
        value = operand(0);
      } else if (IsUnaryOp(instruction.op)) {
        value = EmitUnaryOp(instruction.op, operand(0), resultName, intType,
                            builder);
      } else if (IsBinaryOp(instruction.op)) {
        value = EmitBinaryOp(instruction.op, operand(0), operand(1), resultName,
                             intType, builder, module.get());
      } else if (IsCompoundStoreOp(instruction.op)) {
        auto current =
            LookupValue(instruction.result, values, slots, intType, builder);
        value = EmitBinaryOp(CompoundStoreToBinaryOp(instruction.op), current,
                             operand(0), resultName, intType, builder,
                             module.get());
      } else if (instruction.op == IrOpcode::Print) {
        auto printedVal = operand(0);
        auto printfType = llvm::FunctionType::get(
            intType, {llvm::PointerType::getUnqual(context)}, true);
        auto printfFunc = module->getOrInsertFunction("printf", printfType);
        llvm::Value *fmtPtr = nullptr;
        if (printedVal->getType()->isPointerTy()) {
          fmtPtr = builder.CreateGlobalString("%s\n", ".printf.str.fmt");
        } else {
          fmtPtr = builder.CreateGlobalString("%d\n", ".printf.fmt");
        }
        builder.CreateCall(printfFunc, {fmtPtr, printedVal}, resultName);
        value = printedVal;
      } else if (instruction.op == IrOpcode::Call) {
        auto target = functionMap.find(instruction.operands[0]);
        if (target == functionMap.end()) {
          throw std::runtime_error("LLVM backend error: unknown function " +
                                   instruction.operands[0] + ".");
        }
        auto *targetFn = target->second;
        auto *targetFnType = targetFn->getFunctionType();
        auto arguments = std::vector<llvm::Value *>();
        for (size_t i = 1; i < instruction.operands.size(); i++) {
          auto *argVal = operand(i);
          size_t paramIdx = i - 1;
          if (paramIdx < targetFnType->getNumParams()) {
            auto *expectedType = targetFnType->getParamType(paramIdx);
            if (argVal->getType() != expectedType) {
              if (expectedType == ptrType && argVal->getType() == intType) {
                argVal = builder.CreateIntToPtr(argVal, ptrType);
              } else if (expectedType == intType &&
                         argVal->getType() == ptrType) {
                argVal = builder.CreatePtrToInt(argVal, intType);
              }
            }
          }
          arguments.push_back(argVal);
        }
        value = builder.CreateCall(targetFn, arguments, resultName);
      } else {
        throw std::runtime_error(
            "LLVM backend error: unsupported instruction " +
            IrOpcodeToString(instruction.op) + ".");
      }

      if (!instruction.result.empty()) {
        auto canonical = CanonicalName(instruction.result);
        auto slot = slots.find(canonical);
        if (slot != slots.end()) {
          builder.CreateStore(value, slot->second.allocaInst);
        } else {
          values[instruction.result] = value;
          values[canonical] = value;
        }
      }
    }

    if (!HasTerminator(builder.GetInsertBlock())) {
      if (function.returnValue == kIrVoidValue) {
        builder.CreateRet(llvm::ConstantInt::get(intType, 0));
      } else {
        auto retVal =
            LookupValue(function.returnValue, values, slots, intType, builder);
        if (function.name == kIrMainFunctionName &&
            retVal->getType()->isPointerTy()) {
          builder.CreateRet(llvm::ConstantInt::get(intType, 0));
        } else if (llvmFunction->getReturnType() == intType &&
                   retVal->getType()->isPointerTy()) {
          builder.CreateRet(builder.CreatePtrToInt(retVal, intType));
        } else if (llvmFunction->getReturnType() == ptrType &&
                   retVal->getType() == intType) {
          builder.CreateRet(builder.CreateIntToPtr(retVal, ptrType));
        } else {
          builder.CreateRet(retVal);
        }
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
