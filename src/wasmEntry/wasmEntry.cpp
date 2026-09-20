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
char *ShowSyntaxTree();
char *ShowIR();
}

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

  auto syntaxTree = SyntaxTree::Parse(s);
  auto binder = std::make_unique<Binder>();
  auto boundStatement = binder->BindCompilationUnit(syntaxTree->Root());

  if (showIR) {
    IrLowerer lowerer;
    auto module = lowerer.Lower(boundStatement.get());

    std::stringstream irStream;
    IrPrinter::Print(module, irStream);
    SetBuffer(g_showIR, irStream.str());
  }

  auto eval = std::make_unique<Evaluator>(std::move(boundStatement));

  std::stringstream outputStream;
  outputStream << eval->Evaluate() << std::endl;
  std::string outputStr = outputStream.str();
  SetBuffer(g_ReturnBuffer, outputStr);

  if (showSyntaxTree) {
    std::stringstream syntaxTreeStream;
    syntaxTree->PrintTree(syntaxTreeStream);

    std::string outputSyntaxTree = syntaxTreeStream.str();
    SetBuffer(g_showSyntaxTree, outputSyntaxTree);
  }

  return g_ReturnBuffer;
}
