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

TEST_CASE("Fuzzer Crash Regressions (Issues #85 and #86)") {
  SUBCASE("IrParser handles non-ASCII bytes and CRLF trimming (#85)") {
    const std::string crashInput(
        "func @add(%%)\x8c }\r\nfunc @main( ){\xff\xff\xff\r\n}\r\n", 40);
    IrParser parser;
    auto mod = parser.Parse(crashInput);
    IrOptimizer::OptimizeInPlace(mod);
    REQUIRE(mod.functions.size() == 2);
  }

  SUBCASE("Parser bounds recursion depth on deeply nested inputs (#86)") {
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
}
