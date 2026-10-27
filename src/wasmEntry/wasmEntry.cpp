// Copyright (c) 2023 F. Lotfi All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include <iostream>
#include <sstream>
#include <stdbool.h>
#include <string.h>
#include <string>

#include <emscripten.h>

#include "Binding/Binder.h"
#include "Evaluator.h"
#include "IR/Ir.h"
#include "Symbol/SymbolTableMgr.h"
#include "Syntax/SyntaxTree.h"

extern "C" {
void InitWarf();
char *RunWarf(char *input, bool showSyntaxTree, bool showIR);
const char *run_warf(const char *input);
const char *emit_warf_ir(const char *input);
char *ShowSyntaxTree();
char *ShowIR();
}

namespace {
struct CoutRedirectGuard {
  std::streambuf *oldBuf;
  explicit CoutRedirectGuard(std::streambuf *newBuf)
      : oldBuf(std::cout.rdbuf(newBuf)) {}
  ~CoutRedirectGuard() { std::cout.rdbuf(oldBuf); }
};
} // namespace

char *g_ReturnBuffer = nullptr;
char *g_showSyntaxTree = nullptr;
char *g_showIR = nullptr;

static void SetBuffer(char *&buffer, const std::string &value) {
  if (buffer) {
    delete[] buffer;
    buffer = nullptr;
  }

  buffer = new char[value.size() + 1];
  strcpy(buffer, value.c_str());
}

// EMSCRIPTEN_KEEPALIVE
void InitWarf() { SymbolTableMgr::init(); }

// EMSCRIPTEN_KEEPALIVE
char *ShowSyntaxTree() { return g_showSyntaxTree; }

// EMSCRIPTEN_KEEPALIVE
char *ShowIR() { return g_showIR; }

// EMSCRIPTEN_KEEPALIVE
char *RunWarf(char *input, bool showSyntaxTree, bool showIR) {

  std::string s(input);

  SetBuffer(g_ReturnBuffer, "");
  SetBuffer(g_showSyntaxTree, "");
  SetBuffer(g_showIR, "");

  SymbolTableMgr::init();
  auto syntaxTree = SyntaxTree::Parse(s);

  if (showSyntaxTree) {
    std::stringstream syntaxTreeStream;
    syntaxTree->PrintTree(syntaxTreeStream);
    SetBuffer(g_showSyntaxTree, syntaxTreeStream.str());
  }

  auto binder = std::make_unique<Binder>();
  std::unique_ptr<BoundStatementNode> boundStatement;
  std::stringstream outputStream;

  try {
    if (syntaxTree->Errors().empty()) {
      boundStatement = binder->BindCompilationUnit(syntaxTree->Root());
    }
  } catch (const std::exception &error) {
    outputStream << error.what() << std::endl;
  }

  if (!syntaxTree->Errors().empty() || !binder->Errors().empty()) {
    for (const auto &error : syntaxTree->Errors()) {
      outputStream << error << std::endl;
    }
    for (const auto &error : binder->Errors()) {
      outputStream << error << std::endl;
    }
    SetBuffer(g_ReturnBuffer, outputStream.str());
    return g_ReturnBuffer;
  }

  if (!boundStatement) {
    SetBuffer(g_ReturnBuffer, outputStream.str());
    return g_ReturnBuffer;
  }

  try {
    if (showIR) {
      IrLowerer lowerer;
      auto module = lowerer.Lower(boundStatement.get());

      std::stringstream irStream;
      IrPrinter::Print(module, irStream);
      SetBuffer(g_showIR, irStream.str());
    }

    auto eval = std::make_unique<Evaluator>(std::move(boundStatement));
    Value result;
    {
      CoutRedirectGuard redirect(outputStream.rdbuf());
      result = eval->Evaluate();
    }
    outputStream << result << std::endl;
  } catch (const std::exception &error) {
    outputStream << error.what() << std::endl;
  }

  SetBuffer(g_ReturnBuffer, outputStream.str());
  return g_ReturnBuffer;
}

EMSCRIPTEN_KEEPALIVE
const char *run_warf(const char *input) {
  return RunWarf(const_cast<char *>(input ? input : ""), false, false);
}

EMSCRIPTEN_KEEPALIVE
const char *emit_warf_ir(const char *input) {
  std::string s(input ? input : "");
  SetBuffer(g_showIR, "");

  SymbolTableMgr::init();
  auto syntaxTree = SyntaxTree::Parse(s);
  auto binder = std::make_unique<Binder>();
  std::unique_ptr<BoundStatementNode> boundStatement;
  std::stringstream outStream;

  try {
    if (syntaxTree->Errors().empty()) {
      boundStatement = binder->BindCompilationUnit(syntaxTree->Root());
    }
  } catch (const std::exception &error) {
    outStream << error.what() << std::endl;
  }

  if (!syntaxTree->Errors().empty() || !binder->Errors().empty()) {
    for (const auto &error : syntaxTree->Errors()) {
      outStream << error << std::endl;
    }
    for (const auto &error : binder->Errors()) {
      outStream << error << std::endl;
    }
    SetBuffer(g_showIR, outStream.str());
    return g_showIR;
  }

  if (!boundStatement) {
    SetBuffer(g_showIR, outStream.str());
    return g_showIR;
  }

  try {
    IrLowerer lowerer;
    auto module = lowerer.Lower(boundStatement.get());
    IrPrinter::Print(module, outStream);
  } catch (const std::exception &error) {
    outStream << error.what() << std::endl;
  }

  SetBuffer(g_showIR, outStream.str());
  return g_showIR;
}
