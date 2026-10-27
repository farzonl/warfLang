/*
 * Copyright (c) 2026 F. Lotfi All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#pragma once

#include "Binding/BoundExpressionNode.h"
#include "IR/IrKind.h"
#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class BoundCallExpressionNode;
class VariableSymbol;

struct IrInstruction {
  std::string result;
  IrOpcode op;
  std::vector<std::string> operands;
};

struct IrFunction {
  std::string name;
  std::vector<std::string> parameters;
  std::vector<IrInstruction> instructions;
  std::string returnValue;
};

struct IrModule {
  std::vector<IrFunction> functions;
};

class IrLowerer {
public:
  IrModule Lower(BoundStatementNode *root);

private:
  static constexpr int32_t kMaxLowerDepth = 64;
  IrModule mModule;
  IrFunction *mCurrentFunction;
  int32_t mTempIndex;
  int32_t mLabelIndex;
  int32_t mLowerDepth = 0;
  std::unordered_set<std::string> mEmittedFunctions;
  std::unordered_map<const VariableSymbol *, std::string> mVariableNames;
  std::unordered_map<std::string, size_t> mVariableNameCounts;
  struct LoopTargets {
    std::string breakLabel;
    std::string continueLabel;
    bool usedContinue = false;
  };
  std::vector<LoopTargets> mLoopStack;

  std::string GetVariableName(const VariableSymbol *variable,
                              const std::string &fallbackName = "");
  void LowerStatement(BoundStatementNode *node);
  std::string LowerExpression(BoundExpressionNode *node);
  void LowerFunction(BoundCallExpressionNode *call);
  void LowerLoop(IrLabelKind condKind, IrLabelKind bodyKind,
                 IrLabelKind endKind, BoundStatementNode *initializer,
                 BoundExpressionNode *condition, BoundStatementNode *body,
                 BoundExpressionNode *increment);
  void LowerDoWhile(BoundStatementNode *body, BoundExpressionNode *condition);
  void Emit(IrOpcode op, std::vector<std::string> operands,
            std::string result = "");
  void EmitAt(size_t index, IrOpcode op, std::vector<std::string> operands,
              std::string result = "");
  void EmitLabel(const std::string &label);
  void EmitBranch(const std::string &targetLabel);
  void EmitCondBranch(const std::string &condition,
                      const std::string &trueLabel,
                      const std::string &falseLabel);
  std::string NewTemp();
};

class IrPrinter {
public:
  static void Print(const IrModule &module, std::ostream &out = std::cout);
};

class IrParser {
public:
  IrModule Parse(const std::string &text);

private:
  static std::string Trim(const std::string &text);
  static std::vector<std::string> SplitWhitespace(const std::string &text);
  static std::vector<std::string> ParseParameters(const std::string &text);
};

class IrInterpreter {
public:
  Value Execute(const IrModule &module);

private:
  const IrModule *mModule;

  Value ExecuteFunction(const IrFunction &function,
                        const std::vector<Value> &arguments);
  Value EvaluateOperand(const std::string &operand,
                        const std::unordered_map<std::string, Value> &values);
  static Value EvaluateUnaryOp(IrOpcode op, Value operand);
  static Value EvaluateBinaryOp(IrOpcode op, Value left, Value right);
  static size_t
  LookupLabel(const std::string &label,
              const std::unordered_map<std::string, size_t> &labelMap);
  const IrFunction *FindFunction(const std::string &name) const;
};
