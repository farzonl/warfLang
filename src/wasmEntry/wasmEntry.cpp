// Copyright (c) 2023 F. Lotfi All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include <iostream>
#include <sstream>
#include <stdbool.h>
#include <string>

#include <emscripten.h>

#include "Binding/Binder.h"
#include "Evaluator.h"
#include "Symbol/SymbolTableMgr.h"
#include "Syntax/SyntaxTree.h"

extern "C" {
void InitWarf();
char *RunWarf(char *input, bool showSyntaxTree);
char *ShowSyntaxTree();
}

char *g_ReturnBuffer = nullptr;
char *g_showSyntaxTree = nullptr;

// EMSCRIPTEN_KEEPALIVE
void InitWarf() { SymbolTableMgr::init(); }

// EMSCRIPTEN_KEEPALIVE
char *ShowSyntaxTree() { return g_showSyntaxTree; }

// EMSCRIPTEN_KEEPALIVE
char *RunWarf(char *input, bool showSyntaxTree) {

  std::string s(input);

  if (g_ReturnBuffer) {
    delete g_ReturnBuffer;
    g_ReturnBuffer = nullptr;
  }

  if (g_showSyntaxTree && showSyntaxTree) {
    delete g_showSyntaxTree;
    g_showSyntaxTree = nullptr;
  }

  auto syntaxTree = SyntaxTree::Parse(s);
  auto binder = std::make_unique<Binder>();
  auto boundExpression = binder->BindExpression(syntaxTree->Root());
  auto eval = std::make_unique<Evaluator>(std::move(boundExpression));

  std::stringstream outputStream;
  outputStream << eval->Evaluate() << std::endl;
  std::string outputStr = outputStream.str();
  g_ReturnBuffer = new char(outputStr.size());
  stpncpy(g_ReturnBuffer, outputStr.c_str(), outputStr.size());

  if (showSyntaxTree) {
    std::stringstream syntaxTreeStream;
    syntaxTree->PrintTree(syntaxTreeStream);

    std::string outputSyntaxTree = syntaxTreeStream.str();
    g_showSyntaxTree = new char(outputSyntaxTree.size());
    stpncpy(g_showSyntaxTree, outputSyntaxTree.c_str(),
            outputSyntaxTree.size());
  }

  return g_ReturnBuffer;
}