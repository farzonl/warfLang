// Copyright (c) 2022 F. Lotfi All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "Binding/Binder.h"
#include "CodeAnalysis/Compilation.h"
#include "CodeAnalysis/Evaluator.h"
#include "DataStructures/TemplateList.h"
#include "IR/Ir.h"
#include "IR/IrOptimizer.h"
#include "Symbol/SymbolTableMgr.h"
#include "Symbol/TextSpan.h"
#include "Syntax/ExpressionStatementSyntaxNode.h"
#include "Syntax/SyntaxTree.h"
#include <doctest/doctest.h>

static ExpressionNode *ParseExpression(SyntaxTree *syntaxTree) {
  auto root = syntaxTree->Root();
  auto statement = root->Statement();
  if (statement->Kind() == SyntaxKind::ExpressionStatement) {
    auto expressionStatement = dynamic_cast<ExpressionStatementSyntaxNode *>(
        const_cast<StatementSyntaxNode *>(statement));
    return const_cast<ExpressionNode *>(expressionStatement->Expression());
  }
  return nullptr;
}

TEST_CASE("TextSpan Line and Column Tracking") {
  const std::string src = "let a = 10\nvar b = 20\na + b";
  // Line 1: "let a = 10" (indices 0..9, '\n' at 10)
  // Line 2: "var b = 20" (indices 11..20, '\n' at 21)
  // Line 3: "a + b"      (indices 22..26)
  REQUIRE(LineColumn{1, 1} == TextSpan::ComputeLineColumn(src, 0));
  REQUIRE(LineColumn{1, 5} == TextSpan::ComputeLineColumn(src, 4));
  REQUIRE(LineColumn{2, 1} == TextSpan::ComputeLineColumn(src, 11));
  REQUIRE(LineColumn{3, 5} == TextSpan::ComputeLineColumn(src, 26));
  REQUIRE(LineColumn{1, 1} != LineColumn{2, 1});

  TextSpan spanSecondLineVar(15, 16); // 'b' on line 2, col 5
  REQUIRE(15 == spanSecondLineVar.Start());
  REQUIRE(1 == spanSecondLineVar.Length());
  REQUIRE(15 == spanSecondLineVar.End());
  REQUIRE(LineColumn{2, 5} == spanSecondLineVar.StartLineColumn(src));
  REQUIRE(LineColumn{2, 5} == spanSecondLineVar.EndLineColumn(src));
  REQUIRE("2:5" == spanSecondLineVar.FormatLocation(src));
}

TEST_CASE("Direct Expression Binding and Evaluation API") {
  SymbolTableMgr::init();
  auto syntaxTree = SyntaxTree::Parse("4 * (1 + 3)");
  auto *expressionNode = ParseExpression(syntaxTree.get());
  REQUIRE(expressionNode != nullptr);

  Binder binder;
  auto boundExpression = binder.BindExpression(expressionNode);
  REQUIRE(binder.Errors().empty());

  Evaluator eval(std::move(boundExpression));
  REQUIRE(eval.Root() != nullptr);
  REQUIRE(16 == eval.Evaluate().asInt());
}

TEST_CASE("Compilation Chain and SymbolTableMgr C++ APIs") {
  SUBCASE("Compilation links previous syntax trees") {
    auto firstTree = SyntaxTree::Parse("a = 1");
    auto *firstTreePtr = firstTree.get();
    auto firstComp = std::make_unique<Compilation>(std::move(firstTree));
    REQUIRE(firstComp->Previous() == nullptr);
    REQUIRE(firstComp->STree() == firstTreePtr);

    auto secondTree = SyntaxTree::Parse("a + 2");
    auto *secondTreePtr = secondTree.get();
    Compilation secondComp(std::move(firstComp), std::move(secondTree));
    REQUIRE(secondComp.Previous() != nullptr);
    REQUIRE(secondComp.Previous()->STree() == firstTreePtr);
    REQUIRE(secondComp.STree() == secondTreePtr);
  }

  SUBCASE("SymbolTableMgr insert, find, and modify") {
    SymbolTableMgr::init();
    Value initialVal;
    initialVal = 10;
    auto var = std::make_shared<VariableSymbol>("cpp_api_var", false,
                                                Value::Type::Number);
    var->SetValue(initialVal);
    REQUIRE(SymbolTableMgr::insert(var));

    auto found = SymbolTableMgr::find("cpp_api_var");
    REQUIRE(found != VariableSymbol::failSymbol());
    REQUIRE(10 == found->GetValue().asInt());

    Value updatedVal;
    updatedVal = 42;
    auto updated = std::make_shared<VariableSymbol>("cpp_api_var", false,
                                                    Value::Type::Number);
    updated->SetValue(updatedVal);
    REQUIRE(SymbolTableMgr::modify(updated, ".global"));
    REQUIRE(42 ==
            SymbolTableMgr::find("cpp_api_var", ".global")->GetValue().asInt());
    REQUIRE(SymbolTableMgr::find("nonexistent_var") ==
            VariableSymbol::failSymbol());
  }

  SUBCASE("TemplateList container operations") {
    TemplateList<int> list({1, 2, 3});
    REQUIRE_FALSE(list.empty());
    REQUIRE(3 == list.size());
    REQUIRE(2 == list[1]);

    std::vector<int> replacement = {10, 20};
    list.assign(replacement.begin(), replacement.end());
    REQUIRE(2 == list.size());
    REQUIRE(10 == list[0]);
    REQUIRE(20 == list[1]);
  }
}

TEST_CASE("Parser Deep Nesting Stack Overflow Regression (Issue #86)") {
  const std::string deepParens =
      std::string(4000, '(') + "1" + std::string(4000, ')');
  auto tree1 = SyntaxTree::Parse(deepParens);
  REQUIRE(tree1 != nullptr);
  REQUIRE_FALSE(tree1->Errors().empty());

  const std::string deepUnary = std::string(4000, '-') + "1";
  auto tree2 = SyntaxTree::Parse(deepUnary);
  REQUIRE(tree2 != nullptr);
  REQUIRE_FALSE(tree2->Errors().empty());

  std::string deepAssign;
  for (int i = 0; i < 2000; ++i) {
    deepAssign += "a = ";
  }
  deepAssign += "1";
  auto tree3 = SyntaxTree::Parse(deepAssign);
  REQUIRE(tree3 != nullptr);
  REQUIRE_FALSE(tree3->Errors().empty());

  const std::string deepBlocks =
      std::string(2000, '{') + "1" + std::string(2000, '}');
  auto tree4 = SyntaxTree::Parse(deepBlocks);
  REQUIRE(tree4 != nullptr);
  REQUIRE_FALSE(tree4->Errors().empty());
}

TEST_CASE("TextSpan operations") {
  TextSpan defaultSpan;
  REQUIRE(defaultSpan.Start() == 0);
  REQUIRE(defaultSpan.End() == -1);
  REQUIRE(defaultSpan.EndPosition() == 0);
  REQUIRE(defaultSpan.Length() == 0);

  TextSpan span(10, 25);
  REQUIRE(span.Start() == 10);
  REQUIRE(span.End() == 24);
  REQUIRE(span.EndPosition() == 25);
  REQUIRE(span.Length() == 15);

  TextSpan same(10, 25);
  TextSpan different(10, 26);
  REQUIRE(span == same);
  REQUIRE(span != different);
  REQUIRE(defaultSpan != span);
}

TEST_CASE("SyntaxNode and SyntaxToken Span and Leaf Queries") {
  auto syntaxTree = SyntaxTree::Parse("let x = 10 + 20;");
  REQUIRE(syntaxTree->Errors().empty());
  auto *root = syntaxTree->Root();
  REQUIRE(root != nullptr);

  TextSpan rootSpan = root->Span();
  REQUIRE(rootSpan.Start() == 0);
  REQUIRE(rootSpan.EndPosition() > 0);
  REQUIRE(rootSpan.Length() > 0);

  auto leaves = root->GetLeafTokens();
  REQUIRE_FALSE(leaves.empty());
  REQUIRE(leaves.front()->Span().Start() == 0);
  REQUIRE(leaves.back()->Span().EndPosition() == rootSpan.EndPosition());
}

TEST_CASE("BoundNode source span propagation") {
  SymbolTableMgr::init();
  auto syntaxTree = SyntaxTree::Parse("let x = 10 + 20;");
  REQUIRE(syntaxTree->Errors().empty());

  Binder binder;
  auto bound = binder.BindCompilationUnit(syntaxTree->Root());
  REQUIRE(binder.Errors().empty());
  REQUIRE(bound != nullptr);

  REQUIRE(bound->Span() == syntaxTree->Root()->Span());
  REQUIRE(bound->Span().Start() == 0);
  REQUIRE(bound->Span().Length() > 0);
}

TEST_CASE("IrInstruction span propagation") {
  SymbolTableMgr::init();
  auto syntaxTree = SyntaxTree::Parse("let x = 10 + 20;");
  REQUIRE(syntaxTree->Errors().empty());

  Binder binder;
  auto bound = binder.BindCompilationUnit(syntaxTree->Root());
  REQUIRE(binder.Errors().empty());

  IrLowerer lowerer;
  auto module = lowerer.Lower(bound.get());
  REQUIRE_FALSE(module.functions.empty());

  const auto &fn = module.functions.front();
  REQUIRE_FALSE(fn.instructions.empty());
  bool hasInstructionWithSpan = false;
  for (const auto &inst : fn.instructions) {
    if (inst.span.Length() > 0) {
      hasInstructionWithSpan = true;
      break;
    }
  }
  REQUIRE(hasInstructionWithSpan);

  IrOptimizer optimizer;
  auto optModule = optimizer.Optimize(module);
  REQUIRE_FALSE(optModule.functions.empty());
  bool hasOptSpan = false;
  for (const auto &inst : optModule.functions.front().instructions) {
    if (inst.span.Length() > 0) {
      hasOptSpan = true;
      break;
    }
  }
  REQUIRE(hasOptSpan);
}

TEST_CASE("Evaluator Step Budget Limit") {
  SUBCASE("Normal execution within step budget succeeds") {
    SymbolTableMgr::init();
    auto syntaxTree = SyntaxTree::Parse("let x = 10 + 20; x;");
    REQUIRE(syntaxTree->Errors().empty());
    Binder binder;
    auto bound = binder.BindCompilationUnit(syntaxTree->Root());
    REQUIRE(binder.Errors().empty());

    Evaluator eval(std::move(bound), 100);
    REQUIRE(eval.MaxSteps() == 100);
    REQUIRE(eval.StepLimit() == 100);
    Value result = eval.Evaluate();
    REQUIRE(result.asInt() == 30);
    REQUIRE(eval.StepCount() > 0);
    REQUIRE(eval.StepCount() <= 100);
  }

  SUBCASE("Infinite while loop exceeds budget and throws EvaluatorError") {
    SymbolTableMgr::init();
    auto syntaxTree = SyntaxTree::Parse("while (true) {}");
    REQUIRE(syntaxTree->Errors().empty());
    Binder binder;
    auto bound = binder.BindCompilationUnit(syntaxTree->Root());
    REQUIRE(binder.Errors().empty());

    Evaluator eval(std::move(bound), 20);
    bool caught = false;
    try {
      eval.Evaluate();
    } catch (const std::runtime_error &e) {
      caught = true;
      std::string msg = e.what();
      REQUIRE(msg.find("EvaluatorError: Step limit exceeded.") != std::string::npos);
    }
    REQUIRE(caught);
    REQUIRE(eval.StepCount() > 20);
  }

  SUBCASE("Infinite for loop exceeds budget and throws EvaluatorError") {
    SymbolTableMgr::init();
    auto syntaxTree = SyntaxTree::Parse("var i = 0 for (; true; i = i + 1) {}");
    REQUIRE(syntaxTree->Errors().empty());
    Binder binder;
    auto bound = binder.BindCompilationUnit(syntaxTree->Root());
    REQUIRE(binder.Errors().empty());

    Evaluator eval(std::move(bound));
    eval.SetStepLimit(30);
    REQUIRE(eval.StepLimit() == 30);
    bool caught = false;
    try {
      eval.Evaluate();
    } catch (const std::runtime_error &e) {
      caught = true;
      std::string msg = e.what();
      REQUIRE(msg.find("EvaluatorError: Step limit exceeded.") != std::string::npos);
    }
    REQUIRE(caught);
  }

  SUBCASE("Infinite do-while loop exceeds budget and throws EvaluatorError") {
    SymbolTableMgr::init();
    auto syntaxTree = SyntaxTree::Parse("do {} while (true);");
    REQUIRE(syntaxTree->Errors().empty());
    Binder binder;
    auto bound = binder.BindCompilationUnit(syntaxTree->Root());
    REQUIRE(binder.Errors().empty());

    Evaluator eval(std::move(bound), 25);
    bool caught = false;
    try {
      eval.Evaluate();
    } catch (const std::runtime_error &e) {
      caught = true;
      std::string msg = e.what();
      REQUIRE(msg.find("EvaluatorError: Step limit exceeded.") != std::string::npos);
    }
    REQUIRE(caught);
  }

  SUBCASE("Infinite recursion exceeds budget and throws EvaluatorError") {
    SymbolTableMgr::init();
    auto syntaxTree = SyntaxTree::Parse("function rec(x) { rec(x); } rec(1);");
    REQUIRE(syntaxTree->Errors().empty());
    Binder binder;
    auto bound = binder.BindCompilationUnit(syntaxTree->Root());
    REQUIRE(binder.Errors().empty());

    Evaluator eval(std::move(bound), 15);
    bool caught = false;
    try {
      eval.Evaluate();
    } catch (const std::runtime_error &e) {
      caught = true;
      std::string msg = e.what();
      REQUIRE(msg.find("EvaluatorError: Step limit exceeded.") != std::string::npos);
    }
    REQUIRE(caught);
  }
}

TEST_CASE("IrInterpreter Step Budget Limit") {
  SUBCASE("Normal IR execution within step budget succeeds") {
    SymbolTableMgr::init();
    auto syntaxTree = SyntaxTree::Parse("let x = 10 + 20; x;");
    REQUIRE(syntaxTree->Errors().empty());
    Binder binder;
    auto bound = binder.BindCompilationUnit(syntaxTree->Root());
    REQUIRE(binder.Errors().empty());

    IrLowerer lowerer;
    auto module = lowerer.Lower(bound.get());

    IrInterpreter interpreter(100);
    REQUIRE(interpreter.StepLimit() == 100);
    Value result = interpreter.Execute(module);
    REQUIRE(result.asInt() == 30);
    REQUIRE(interpreter.StepCount() > 0);
    REQUIRE(interpreter.StepCount() <= 100);
  }

  SUBCASE("Infinite while loop in IR exceeds budget and throws IR error") {
    SymbolTableMgr::init();
    auto syntaxTree = SyntaxTree::Parse("while (true) {}");
    REQUIRE(syntaxTree->Errors().empty());
    Binder binder;
    auto bound = binder.BindCompilationUnit(syntaxTree->Root());
    REQUIRE(binder.Errors().empty());

    IrLowerer lowerer;
    auto module = lowerer.Lower(bound.get());

    IrInterpreter interpreter;
    interpreter.SetStepLimit(25);
    REQUIRE(interpreter.StepLimit() == 25);
    bool caught = false;
    try {
      interpreter.Execute(module);
    } catch (const std::runtime_error &e) {
      caught = true;
      std::string msg = e.what();
      REQUIRE(msg.find("IR error: Step limit exceeded.") != std::string::npos);
    }
    REQUIRE(caught);
    REQUIRE(interpreter.StepCount() > 25);
  }
}
