// Copyright (c) 2026 F. Lotfi All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "Binding/Binder.h"
#include "IR/Ir.h"
#include "IR/IrOptimizer.h"
#include "Symbol/SymbolTableMgr.h"
#include "Syntax/SyntaxTree.h"

void BinderFuzzTest(std::string &line) {
  SymbolTableMgr::init();
  auto syntaxTree = SyntaxTree::Parse(line);
  if (!syntaxTree || !syntaxTree->Errors().empty()) {
    return;
  }

  std::string scopedSource = "{\n" + line + "\n}";
  auto scopedTree = SyntaxTree::Parse(scopedSource);
  if (!scopedTree || !scopedTree->Errors().empty()) {
    return;
  }

  Binder binder;
  auto boundStatement = binder.BindCompilationUnit(scopedTree->Root());
  if (!binder.Errors().empty() || !boundStatement) {
    return;
  }

  IrLowerer lowerer;
  auto module = lowerer.Lower(boundStatement.get());
  IrOptimizer::OptimizeInPlace(module);
}

extern "C" int LLVMFuzzerTestOneInput(const char *cLine, size_t len) {
  if (cLine == nullptr) {
    return -1;
  }
  std::string sLine(cLine, len);
  try {
    BinderFuzzTest(sLine);
  } catch (...) {
    return -1;
  }
  return 0;
}
