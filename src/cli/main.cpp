// Copyright (c) 2022 F. Lotfi All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "Binding/Binder.h"
#include "Evaluator.h"
#include "Syntax/SyntaxTree.h"
#include "Symbol/SymbolTableMgr.h"

int main(int argc, char **argv) {
  std::cout << "warfLang 1.0" << std::endl;
  bool showTree = false;
  while (true) {
    try {
      std::string line;
      std::cout << ">>> ";
      std::getline(std::cin, line);
      if (line == "#showTree") {
        showTree = !showTree;
        std::cout << (showTree ? "Showing parse trees."
                               : "Not showing parse trees")
                  << std::endl;
        continue;
      }
      if (line == "#exit") {
        exit(0);
      }
      SymbolTableMgr::init();
      auto globalScope = SymbolTableMgr::getGlobalScope();
      auto syntaxTree = SyntaxTree::Parse(line);
      globalScope->GetTextSpan()->SetLength(line.size());
      auto binder = std::make_unique<Binder>();
      auto boundExpression = binder->BindExpression(syntaxTree->Root());

      if (showTree) {
        syntaxTree->PrintTree();
      }

      if (binder->Diagnostics().empty()) {
        auto eval = std::make_unique<Evaluator>(std::move(boundExpression));
        if (eval->Errors().size() == 0) {
          Value result = eval->Evaluate();
          std::cout << result << std::endl;
        } else {
          for (auto error : eval->Errors()) {
            std::cerr << error << std::endl;
          }
        }
      } else {
        for (auto error : binder->Diagnostics()) {
          std::cerr << error << std::endl;
        }
      }

    } catch (std::runtime_error &error) {
      std::cerr << error.what() << std::endl;
      std::cin.get();
    } catch (std::exception &error) {
      std::cerr << error.what() << std::endl;
      std::cin.get();
    }
  }
}