/*
 * Copyright (c) 2026 F. Lotfi All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#pragma once

#include "Binding/BoundExpressionNode.h"
#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

class BoundCallExpressionNode;

struct IrInstruction {
  std::string result;
  std::string op;
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
  IrModule mModule;
  IrFunction *mCurrentFunction;
  int32_t mTempIndex;
  std::unordered_set<std::string> mEmittedFunctions;

  void LowerStatement(BoundStatementNode *node);
  std::string LowerExpression(BoundExpressionNode *node);
  void LowerFunction(BoundCallExpressionNode *call);
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
  const IrFunction *FindFunction(const std::string &name) const;
};
