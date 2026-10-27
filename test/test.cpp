// Copyright (c) 2022 F. Lotfi All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "Binding/Binder.h"
#include "CodeAnalysis/Evaluator.h"
#include "IR/Ir.h"
#include "Symbol/SymbolTableMgr.h"
#include "Syntax/ExpressionStatementSyntaxNode.h"
#include "Syntax/SyntaxTree.h"
#include <doctest/doctest.h>
#include <sstream>

ExpressionNode *ParseExpression(SyntaxTree *syntaxTree) {
  auto root = syntaxTree->Root();
  auto statement = root->Statement();
  if (statement->Kind() == SyntaxKind::ExpressionStatement) {
    auto expressionStatement = dynamic_cast<ExpressionStatementSyntaxNode *>(
        const_cast<StatementSyntaxNode *>(statement));
    return const_cast<ExpressionNode *>(expressionStatement->Expression());
  }
  return nullptr;
}

Value testCaseHelper(std::string s) {
  SymbolTableMgr::init();
  auto syntaxTree = SyntaxTree::Parse(s);
  auto expresionNode = ParseExpression(syntaxTree.get());
  auto binder = std::make_unique<Binder>();
  auto boundExpression = binder->BindExpression(expresionNode);
  auto eval = std::make_unique<Evaluator>(std::move(boundExpression));
  return eval->Evaluate();
}

Value testProgramHelper(std::string s) {
  SymbolTableMgr::init();
  auto syntaxTree = SyntaxTree::Parse(s);
  auto binder = std::make_unique<Binder>();
  auto boundStatement = binder->BindCompilationUnit(syntaxTree->Root());
  auto eval = std::make_unique<Evaluator>(std::move(boundStatement));
  return eval->Evaluate();
}

bool testCaseSyntaxErrors(std::string s, std::string errorStr) {
  SymbolTableMgr::init();
  auto syntaxTree = SyntaxTree::Parse(s);
  auto expresionNode = ParseExpression(syntaxTree.get());
  if (!syntaxTree->Errors().empty()) {
    return errorStr == syntaxTree->Errors()[0].Message();
  }
  auto binder = std::make_unique<Binder>();
  std::unique_ptr<BoundExpressionNode> boundExpression;
  try {
    boundExpression = binder->BindExpression(expresionNode);
  } catch (std::runtime_error &error) {
    return errorStr == error.what();
  }
  if (!binder->Errors().empty()) {
    return errorStr == binder->Errors()[0].Message();
  }
#if !defined(Emscripten)
  try {
    auto eval = std::make_unique<Evaluator>(std::move(boundExpression));
    eval->Evaluate();
  } catch (std::runtime_error &error) {
    return errorStr == error.what();
  }
  return false;
#else
  return true;
#endif
}

TEST_CASE("Binary Expression") {
  SUBCASE("negative numbers") { REQUIRE(-1 == testCaseHelper("-1").asInt()); }
  SUBCASE("Simple Addition") { REQUIRE(4 == testCaseHelper("1 + 3").asInt()); }
  SUBCASE("Simple Subtraction") { REQUIRE(1 == testCaseHelper("4-3").asInt()); }
  SUBCASE("Simple Negative Number") {
    REQUIRE(-1 == testCaseHelper("3-4").asInt());
  }
  SUBCASE("Simple Multiplication") {
    REQUIRE(12 == testCaseHelper("4*3").asInt());
  }
  SUBCASE("Multiplication of Negative") {
    REQUIRE(-20 == testCaseHelper("4*-5").asInt());
  }
  SUBCASE("Multiplication of Negative Parentheses") {
    REQUIRE(-20 == testCaseHelper("4*(-5)").asInt());
  }
  SUBCASE("Multiplication of Negative Created in Parentheses") {
    REQUIRE(-8 == testCaseHelper("4*(3-5)").asInt());
  }
  SUBCASE("Simple Division") { REQUIRE(3 == testCaseHelper("9/3").asInt()); }
  SUBCASE("Comparison operators") {
    REQUIRE(testCaseHelper("1 < 2").asBool());
    REQUIRE(testCaseHelper("2 <= 2").asBool());
    REQUIRE_FALSE(testCaseHelper("2 > 2").asBool());
    REQUIRE(testCaseHelper("2 >= 2").asBool());
  }
  SUBCASE("Simple PMDAS Ordering") {
    REQUIRE(7 == testCaseHelper("4*1+3").asInt());
  }
  SUBCASE("Simple Parentheses") {
    REQUIRE(16 == testCaseHelper("4*(1+3)").asInt());
  }
  SUBCASE("Negative Parentheses") {
    REQUIRE(-4 == testCaseHelper("-(1+3)").asInt());
  }

  SUBCASE("Binary Expression Integer Bitwise OR") {
    REQUIRE(3 == testCaseHelper("1 | 2").asInt());
    REQUIRE(3 == testCaseHelper("3 | 1").asInt());
    REQUIRE(3 == testCaseHelper("3 | 2").asInt());
  }

  SUBCASE("Binary Expression Integer Bitwise And") {
    REQUIRE(0 == testCaseHelper("1 & 2").asInt());
    REQUIRE(1 == testCaseHelper("3 & 1").asInt());
    REQUIRE(2 == testCaseHelper("3 & 2").asInt());
  }
  // A B
  // 0 0 0
  // 0 1 1
  // 1 0 1
  // 1 1 0
  SUBCASE("Binary Expression Integer Bitwise XOR") {
    REQUIRE(3 == testCaseHelper("1 ^ 2").asInt());
    REQUIRE(2 == testCaseHelper("3 ^ 1").asInt());
    REQUIRE(1 == testCaseHelper("3 ^ 2").asInt());
  }
}

TEST_CASE("Comments") {
  SUBCASE("trailing line comment is ignored") {
    REQUIRE(4 == testCaseHelper("1 + 3 // trailing comment").asInt());
  }
  SUBCASE("comment does not need a trailing newline") {
    REQUIRE(4 == testCaseHelper("1 + 3 //").asInt());
  }
}

TEST_CASE("Boolean Expression") {
  SUBCASE("types defined") {
    REQUIRE(testCaseHelper("true").asBool());
    REQUIRE_FALSE(testCaseHelper("false").asBool());
  }
  SUBCASE("Unary Expression Boolean Logical Negation") {
    REQUIRE(testCaseHelper("!false").asBool());
    REQUIRE_FALSE(testCaseHelper("!true").asBool());
  }
  SUBCASE("Binary Expression Boolean Logical OR") {
    REQUIRE(testCaseHelper("false || true").asBool());
    REQUIRE(testCaseHelper("true || false").asBool());
    REQUIRE(testCaseHelper("true || true").asBool());
    REQUIRE_FALSE(testCaseHelper("false || false").asBool());

    // Logical negation on the right hand
    REQUIRE_FALSE(testCaseHelper("false || !true").asBool());
    REQUIRE(testCaseHelper("true || !false").asBool());
    REQUIRE(testCaseHelper("false || !false").asBool());

    // Logical negation on the left hand
    REQUIRE(testCaseHelper("!false || true").asBool());
    REQUIRE_FALSE(testCaseHelper("!true || false").asBool());
    REQUIRE(testCaseHelper("!true || true").asBool());
  }
  SUBCASE("Binary Expression Boolean Logical AND") {
    REQUIRE_FALSE(testCaseHelper("false && true").asBool());
    REQUIRE_FALSE(testCaseHelper("true && false").asBool());
    REQUIRE_FALSE(testCaseHelper("false && false").asBool());
    REQUIRE(testCaseHelper("true && true").asBool());

    // Logical negation on the right hand
    REQUIRE(testCaseHelper("true && !false").asBool());
    REQUIRE_FALSE(testCaseHelper("false && !false").asBool());
    REQUIRE_FALSE(testCaseHelper("false && !true").asBool());

    // Logical negation on the left hand
    REQUIRE(testCaseHelper("!false && true").asBool());
    REQUIRE_FALSE(testCaseHelper("!true && false").asBool());
    REQUIRE_FALSE(testCaseHelper("!true && true").asBool());
  }
  SUBCASE("Binary Expression Boolean Equality") {
    REQUIRE(testCaseHelper("true == true").asBool());
    REQUIRE(testCaseHelper("false == false").asBool());
    REQUIRE(testCaseHelper("1 == 1").asBool());
    REQUIRE(testCaseHelper("255 == 255").asBool());

    REQUIRE_FALSE(testCaseHelper("true == false").asBool());
    REQUIRE_FALSE(testCaseHelper("!true == true").asBool());
    REQUIRE_FALSE(testCaseHelper("false == true").asBool());
    REQUIRE_FALSE(testCaseHelper("1 == 2").asBool());
  }

  SUBCASE("Binary Expression Boolean inEquality") {
    REQUIRE_FALSE(testCaseHelper("true != true").asBool());
    REQUIRE_FALSE(testCaseHelper("false != false").asBool());
    REQUIRE_FALSE(testCaseHelper("1 != 1").asBool());
    REQUIRE_FALSE(testCaseHelper("255 != 255").asBool());

    REQUIRE(testCaseHelper("true != false").asBool());
    REQUIRE(testCaseHelper("!true != true").asBool());
    REQUIRE(testCaseHelper("false != true").asBool());
    REQUIRE(testCaseHelper("1 != 2").asBool());
    REQUIRE(testCaseHelper("1 != 111").asBool());
  }

  SUBCASE("Nested Binary Expression Boolean Logical AND") {
    REQUIRE(testCaseHelper("1==1 && true").asBool());
    REQUIRE(testCaseHelper("1==1 && 2==2").asBool());

    REQUIRE_FALSE(testCaseHelper("1==1 && false").asBool());
    REQUIRE_FALSE(testCaseHelper("1==1 && 2==3").asBool());
    REQUIRE_FALSE(testCaseHelper("false && 2==2").asBool());
    REQUIRE_FALSE(testCaseHelper("1==2 && 2==2").asBool());
    REQUIRE_FALSE(testCaseHelper("1==2 && 2==3").asBool());
  }

  SUBCASE("Nested Binary Expression Boolean Logical OR") {
    REQUIRE(testCaseHelper("1==1 || true").asBool());
    REQUIRE(testCaseHelper("1==1 || 2==2").asBool());
    REQUIRE(testCaseHelper("1==1 || false").asBool());
    REQUIRE(testCaseHelper("1==1 || 2==3").asBool());
    REQUIRE(testCaseHelper("false || 2==2").asBool());
    REQUIRE(testCaseHelper("1==2 || 2==2").asBool());

    REQUIRE_FALSE(testCaseHelper("1==2 || 2==3").asBool());
  }

  SUBCASE("Binary Expression Boolean LessThan") {
    REQUIRE(testCaseHelper("1 < 2").asBool());
    REQUIRE_FALSE(testCaseHelper("1 < -1").asBool());
    REQUIRE(1 == testCaseHelper("a1 =  1").asInt());
    REQUIRE(-1 == testCaseHelper("a2 = -1").asInt());
    REQUIRE_FALSE(testCaseHelper("a1 < a2").asBool());
    REQUIRE_FALSE(testCaseHelper("a1 + 1 < a2 + 2").asBool());
  }

  SUBCASE("Binary Expression Boolean GreaterThan") {
    REQUIRE_FALSE(testCaseHelper("1 > 2").asBool());
    REQUIRE(testCaseHelper("1 > -1").asBool());
    REQUIRE(1 == testCaseHelper("a1 =  1").asInt());
    REQUIRE(-1 == testCaseHelper("a2 = -1").asInt());
    REQUIRE(testCaseHelper("a1 > a2").asBool());
    REQUIRE(testCaseHelper("a1 + 1 > a2 + 2").asBool());
  }

  SUBCASE("Binary Expression Boolean LessThanOrEqualTo") {
    REQUIRE(testCaseHelper("1 <= 2").asBool());
    REQUIRE(testCaseHelper("1 <= 1").asBool());
    REQUIRE_FALSE(testCaseHelper("1 <= -1").asBool());
    REQUIRE(1 == testCaseHelper("a1 =  1").asInt());
    REQUIRE(-1 == testCaseHelper("a2 = -1").asInt());
    REQUIRE_FALSE(testCaseHelper("a1 <= a2").asBool());
    REQUIRE_FALSE(testCaseHelper("a1 + 1 <= a2 + 2").asBool());
  }

  SUBCASE("Binary Expression Boolean GreaterThanOrEqualTo") {
    REQUIRE_FALSE(testCaseHelper("1 >= 2").asBool());
    REQUIRE(testCaseHelper("1 >= 1").asBool());
    REQUIRE(testCaseHelper("1 >= -1").asBool());
    REQUIRE(1 == testCaseHelper("a1 =  1").asInt());
    REQUIRE(-1 == testCaseHelper("a2 = -1").asInt());
    REQUIRE(testCaseHelper("a1 >= a2").asBool());
    REQUIRE(testCaseHelper("a1 + 1 >= a2 + 2").asBool());
  }
}

TEST_CASE("Statements") {
  SymbolTableMgr::init();
  auto syntaxTree = SyntaxTree::Parse("value = 4 value = value + 1 value");
  REQUIRE(syntaxTree->Errors().empty());

  auto binder = std::make_unique<Binder>();
  auto boundStatement = binder->BindCompilationUnit(syntaxTree->Root());
  REQUIRE(binder->Errors().empty());

  auto evaluator = std::make_unique<Evaluator>(std::move(boundStatement));
  REQUIRE(5 == evaluator->Evaluate().asInt());
}

TEST_CASE("Block scopes") {
  SymbolTableMgr::init();

  auto parentSyntax =
      SyntaxTree::Parse("scope_outer = 1 { scope_outer = 2 } scope_outer");
  auto parentBinder = std::make_unique<Binder>();
  auto parentStatement =
      parentBinder->BindCompilationUnit(parentSyntax->Root());
  auto parentEvaluator =
      std::make_unique<Evaluator>(std::move(parentStatement));
  REQUIRE(2 == parentEvaluator->Evaluate().asInt());

  auto localSyntax = SyntaxTree::Parse("{ scope_inner = 3 }");
  auto localBinder = std::make_unique<Binder>();
  auto localStatement = localBinder->BindCompilationUnit(localSyntax->Root());
  auto localEvaluator = std::make_unique<Evaluator>(std::move(localStatement));
  REQUIRE(3 == localEvaluator->Evaluate().asInt());

  auto lookupSyntax = SyntaxTree::Parse("scope_inner");
  auto lookupBinder = std::make_unique<Binder>();
  REQUIRE_THROWS(lookupBinder->BindCompilationUnit(lookupSyntax->Root()));
}

TEST_CASE("Control flow statements") {
  SUBCASE("if and else") {
    SymbolTableMgr::init();
    auto syntax = SyntaxTree::Parse(
        "value = 0 if (1 < 2) { value = 4 } else { value = 5 } value");
    auto binder = std::make_unique<Binder>();
    auto statement = binder->BindCompilationUnit(syntax->Root());
    REQUIRE(binder->Errors().empty());
    auto evaluator = std::make_unique<Evaluator>(std::move(statement));
    REQUIRE(4 == evaluator->Evaluate().asInt());
  }
  SUBCASE("while") {
    SymbolTableMgr::init();
    auto syntax =
        SyntaxTree::Parse("value = 0 while (value < 3) { value += 1 } value");
    auto binder = std::make_unique<Binder>();
    auto statement = binder->BindCompilationUnit(syntax->Root());
    auto evaluator = std::make_unique<Evaluator>(std::move(statement));
    REQUIRE(3 == evaluator->Evaluate().asInt());
  }
  SUBCASE("for") {
    SymbolTableMgr::init();
    auto syntax = SyntaxTree::Parse("value = 0 for (index = 0; index < 3; "
                                    "index += 1) { value += 2 } value");
    auto binder = std::make_unique<Binder>();
    auto statement = binder->BindCompilationUnit(syntax->Root());
    auto evaluator = std::make_unique<Evaluator>(std::move(statement));
    REQUIRE(6 == evaluator->Evaluate().asInt());
  }
}

TEST_CASE("Function calls") {
  SUBCASE("undefined function reports a binder error") {
    REQUIRE(testCaseSyntaxErrors(
        "missing()",
        "BinderError: Undefined function missing Starting at Position 0 Ending "
        "at: 6."));
  }
  SUBCASE("calls user-defined foo") {
    REQUIRE(10 == testProgramHelper("function foo(a) { a } foo(10)").asInt());
  }
}

TEST_CASE("Assignment Expression") {
  SUBCASE("Simple assignment of Number") {
    REQUIRE(1 == testCaseHelper("a1 = 1").asInt());
    REQUIRE(1 == testCaseHelper("a1").asInt());
  }
  SUBCASE("Simple assignment of Bool") {
    REQUIRE(testCaseHelper("b1 = true").asBool());
    REQUIRE(testCaseHelper("b1").asBool());
    REQUIRE_FALSE(testCaseHelper("b2 = false").asBool());
    REQUIRE_FALSE(testCaseHelper("b2").asBool());
  }

  SUBCASE("Assignment of binary expression") {
    REQUIRE(3 == testCaseHelper("a1 = 3*(3+2)/5").asInt());
    REQUIRE(3 == testCaseHelper("a1").asInt());
    REQUIRE(testCaseHelper("b1 = 3*(3+2)/5 == 3").asBool());
    REQUIRE(testCaseHelper("b1").asBool());
    REQUIRE(testCaseHelper("b2 = 3*(3+2)/5 == 3").asBool());
    REQUIRE(testCaseHelper("b1").asBool());
  }
}

TEST_CASE("Number Unary expressions using IdentifierExpressions") {
  SUBCASE("Unary Expression using Numeric negation") {
    REQUIRE(1 == testCaseHelper("b1 = 1").asInt());
    REQUIRE(-1 == testCaseHelper("b2 = -1").asInt());

    REQUIRE(-1 == testCaseHelper("-b1").asInt());
    REQUIRE(1 == testCaseHelper("-b2").asInt());
  }
  SUBCASE("Unary Expression using Numeric identity") {
    REQUIRE(1 == testCaseHelper("b1 = 1").asInt());
    REQUIRE(-1 == testCaseHelper("b2 = -1").asInt());

    REQUIRE(1 == testCaseHelper("+b1").asInt());
    REQUIRE(-1 == testCaseHelper("+b2").asInt());
  }
}

TEST_CASE("Boolean Unary expressions using IdentifierExpressions") {
  SUBCASE("Unary Expression using Logical negation") {
    REQUIRE(testCaseHelper("b1 = true").asBool());
    REQUIRE_FALSE(testCaseHelper("b2 = false").asBool());

    REQUIRE_FALSE(testCaseHelper("!b1").asBool());
    REQUIRE(testCaseHelper("!b2").asBool());
  }
}

TEST_CASE("Boolean Binary expressions using IdentifierExpressions") {
  SUBCASE("Binary Expression using AND") {
    REQUIRE(testCaseHelper("b1 = true").asBool());
    REQUIRE(testCaseHelper("b2 = true").asBool());
    REQUIRE_FALSE(testCaseHelper("b3 = false").asBool());
    REQUIRE(testCaseHelper("b1 && b2").asBool());
    REQUIRE_FALSE(testCaseHelper("b1 && b3").asBool());
  }

  SUBCASE("Binary Expression using OR") {
    REQUIRE(testCaseHelper("b1 = true").asBool());
    REQUIRE_FALSE(testCaseHelper("b2 = false").asBool());
    REQUIRE_FALSE(testCaseHelper("b3 = false").asBool());

    REQUIRE(testCaseHelper("b1 || b2").asBool());
    REQUIRE(testCaseHelper("b2 || b1").asBool());
    REQUIRE_FALSE(testCaseHelper("b2 || b3").asBool());
  }

  SUBCASE("Binary Expression Equality") {
    REQUIRE(testCaseHelper("b1 = true").asBool());
    REQUIRE(testCaseHelper("b2 = true").asBool());
    REQUIRE_FALSE(testCaseHelper("b3 = false").asBool());

    REQUIRE(testCaseHelper("b1 == b2").asBool());
    REQUIRE(testCaseHelper("b1 == b1").asBool());
    REQUIRE_FALSE(testCaseHelper("b1 == b3").asBool());
  }

  SUBCASE("Binary Expression inEquality") {
    REQUIRE(testCaseHelper("b1 = true").asBool());
    REQUIRE(testCaseHelper("b2 = true").asBool());
    REQUIRE_FALSE(testCaseHelper("b3 = false").asBool());

    REQUIRE_FALSE(testCaseHelper("b1 != b2").asBool());
    REQUIRE_FALSE(testCaseHelper("b1 != b1").asBool());
    REQUIRE(testCaseHelper("b1 != b3").asBool());
  }
}

TEST_CASE("Compound Assignment Expressions") {
  SUBCASE("Assignment and Add a Number") {
    REQUIRE(1 == testCaseHelper("_a = 1").asInt());
    REQUIRE(2 == testCaseHelper("_a += 1").asInt());
    REQUIRE(1 == testCaseHelper("_a = 1").asInt());
    REQUIRE(5 == testCaseHelper("_a += 4").asInt());
    REQUIRE(5 == testCaseHelper("_a += 0").asInt());
    REQUIRE(0 == testCaseHelper("_a = 0").asInt());
    REQUIRE(0 == testCaseHelper("_a += 0").asInt());
  }
  SUBCASE("Assignment and Subtract a Number") {
    REQUIRE(1 == testCaseHelper("a = 1").asInt());
    REQUIRE(0 == testCaseHelper("a -= 1").asInt());
  }
  SUBCASE("Assignment and Multipy a Number") {
    REQUIRE(2 == testCaseHelper("a = 2").asInt());
    REQUIRE(4 == testCaseHelper("a *= 2").asInt());
  }
  SUBCASE("Assignment and Divide a Number") {
    REQUIRE(2 == testCaseHelper("a = 2").asInt());
    REQUIRE(1 == testCaseHelper("a /= 2").asInt());
  }
  SUBCASE("Assignment and Bitwise And a Number") {
    REQUIRE(3 == testCaseHelper("a = 3").asInt());
    REQUIRE(2 == testCaseHelper("a &= 2").asInt());
  }
  SUBCASE("Assignment and Bitwise OR a Number") {
    REQUIRE(2 == testCaseHelper("a = 2").asInt());
    REQUIRE(3 == testCaseHelper("a |= 1").asInt());
  }
  SUBCASE("Assignment and Bitwise XOR a Number") {
    REQUIRE(1 == testCaseHelper("a = 1").asInt());
    REQUIRE(3 == testCaseHelper("a ^= 2").asInt());
  }
}
TEST_CASE("Runtime Exceptions") {
  SUBCASE("Lexer Errors") {
    REQUIRE(testCaseSyntaxErrors(
        "2147483648",
        "LexerError: The number 2147483648 is a Numeric overflow."));
    REQUIRE(
        testCaseSyntaxErrors("1 ? 2", "LexerError: Bad character input: ?."));
  }
  SUBCASE("Parser Errors") {
    REQUIRE(testCaseSyntaxErrors(
        "a = ((1 + 2)", "ParserError: Unexpected token: < EndOfFileToken > "
                        "Expected: < CloseParenthesisToken >"));
  }
  SUBCASE("Bind Error") {
    REQUIRE(testCaseSyntaxErrors(
        "!1", "BinderError: Unary operator ! is not defined for type Number."));
    testCaseSyntaxErrors("1 == true", "BinderError: Binary operator == is not "
                                      "defined for types Number and Boolean.");
    testCaseSyntaxErrors(
        "a += 1",
        "BinderError: Undefined name a Starting at Position 0 Ending at: 1.");
    testCaseSyntaxErrors(
        "b",
        "BinderError: Undefined name a Starting at Position 0 Ending at: 1.");
  }
  SUBCASE("Exceptions") {
    REQUIRE(testCaseSyntaxErrors("1 / 0", "Logial error: Divide by zero."));
  }
}

static Value testIrRoundTripHelper(const std::string &source) {
  SymbolTableMgr::init();
  auto syntaxTree = SyntaxTree::Parse(source);
  auto binder = std::make_unique<Binder>();
  auto boundStatement = binder->BindCompilationUnit(syntaxTree->Root());
  IrLowerer lowerer;
  auto lowered = lowerer.Lower(boundStatement.get());
  std::stringstream irStream;
  IrPrinter::Print(lowered, irStream);
  IrParser parser;
  auto parsed = parser.Parse(irStream.str());
  IrInterpreter interpreter;
  return interpreter.Execute(parsed);
}

TEST_CASE("IR Control Flow and Compound Assignments") {
  SUBCASE("if and else") {
    REQUIRE(4 == testIrRoundTripHelper(
                     "value = 0 if (1 < 2) { value = 4 } else { value = 5 } "
                     "value")
                     .asInt());
    REQUIRE(5 == testIrRoundTripHelper(
                     "value = 0 if (2 < 1) { value = 4 } else { value = 5 } "
                     "value")
                     .asInt());
    REQUIRE(20 == testIrRoundTripHelper(
                      "function max(a, b) { if (a > b) { a } else { b } } "
                      "max(10, 20)")
                      .asInt());
  }
  SUBCASE("while and for loops") {
    REQUIRE(3 == testIrRoundTripHelper(
                     "value = 0 while (value < 3) { value += 1 } value")
                     .asInt());
    REQUIRE(6 == testIrRoundTripHelper(
                     "value = 0 for (index = 0; index < 3; index += 1) { "
                     "value += 2 } value")
                     .asInt());
    const std::string multilineWhileWithSemicolon = "NUM = 10\n"
                                                    "i = 0\n"
                                                    "a = 0\n"
                                                    "while(i < NUM) {\n"
                                                    "  a = 3 * i + a;\n"
                                                    "  i = i + 1\n"
                                                    "}\n"
                                                    "a";
    REQUIRE(135 == testProgramHelper(multilineWhileWithSemicolon).asInt());
    REQUIRE(135 == testIrRoundTripHelper(multilineWhileWithSemicolon).asInt());
  }
  SUBCASE("compound assignments") {
    const std::string program =
        "a = 4 a += 2 a -= 1 a *= 4 a /= 2 a &= 7 a |= 8 a ^= 5 a";
    REQUIRE(15 == testProgramHelper(program).asInt());
    REQUIRE(15 == testIrRoundTripHelper(program).asInt());
  }
}

TEST_CASE("Functions, Return Statements, Types, and Recursion") {
  SUBCASE("explicit return and early return") {
    const std::string program =
        "function abs(x: int): int { if (x < 0) { return -x } return x } "
        "abs(-7) + abs(5)";
    REQUIRE(12 == testProgramHelper(program).asInt());
    REQUIRE(12 == testIrRoundTripHelper(program).asInt());
  }
  SUBCASE("boolean return and composition") {
    const std::string program =
        "function isPositive(x: int): bool { return x > 0 } "
        "res = 0 if (isPositive(3) && !isPositive(-1)) { res = 42 } res";
    REQUIRE(42 == testProgramHelper(program).asInt());
    REQUIRE(42 == testIrRoundTripHelper(program).asInt());
  }
  SUBCASE("recursion: factorial and fibonacci") {
    const std::string factProg =
        "function fact(n: int): int { if (n <= 1) { return 1 } return n * "
        "fact(n - 1) } fact(5)";
    REQUIRE(120 == testProgramHelper(factProg).asInt());
    REQUIRE(120 == testIrRoundTripHelper(factProg).asInt());

    const std::string fibProg =
        "function fib(n: int): int { if (n <= 1) { return n } return fib(n - "
        "1) + fib(n - 2) } fib(6)";
    REQUIRE(8 == testProgramHelper(fibProg).asInt());
    REQUIRE(8 == testIrRoundTripHelper(fibProg).asInt());
  }
  SUBCASE("wrong argument type diagnostic") {
    SymbolTableMgr::init();
    auto syntaxTree = SyntaxTree::Parse(
        "function inc(x: int): int { return x + 1 } inc(true)");
    auto binder = std::make_unique<Binder>();
    binder->BindCompilationUnit(syntaxTree->Root());
    REQUIRE_FALSE(binder->Errors().empty());
    REQUIRE(binder->Errors()[0].Message() ==
            "BinderError: Parameter x requires a value of type Number "
            "but was given a value of type Boolean.");
  }
}

TEST_CASE("Loop Control (break, continue, do-while) and let Immutability") {
  SUBCASE("break and continue in while loop") {
    const std::string program = "i = 0 sum = 0 "
                                "while (i < 10) { "
                                "  i += 1 "
                                "  if (i == 3) { continue } "
                                "  if (i == 6) { break } "
                                "  sum += i "
                                "} sum";
    // i = 1 (sum=1), i = 2 (sum=3), i = 3 (continue), i = 4 (sum=7), i = 5
    // (sum=12), i = 6 (break)
    REQUIRE(12 == testProgramHelper(program).asInt());
    REQUIRE(12 == testIrRoundTripHelper(program).asInt());
  }
  SUBCASE("break and continue in for loop") {
    const std::string program = "sum = 0 "
                                "for (var k = 0; k < 10; k += 1) { "
                                "  if (k == 2) { continue } "
                                "  if (k == 5) { break } "
                                "  sum += k "
                                "} sum";
    // k = 0 (sum=0), k = 1 (sum=1), k = 2 (continue), k = 3 (sum=4), k = 4
    // (sum=8), k = 5 (break)
    REQUIRE(8 == testProgramHelper(program).asInt());
    REQUIRE(8 == testIrRoundTripHelper(program).asInt());
  }
  SUBCASE("do-while loop with break and continue") {
    const std::string program = "n = 0 total = 0 "
                                "do { "
                                "  n += 1 "
                                "  if (n == 2) { continue } "
                                "  total += 10 "
                                "  if (n == 4) { break } "
                                "} while (n < 10) "
                                "total";
    // n=1 (+10), n=2 (continue), n=3 (+10), n=4 (+10, break) -> total = 30
    REQUIRE(30 == testProgramHelper(program).asInt());
    REQUIRE(30 == testIrRoundTripHelper(program).asInt());
  }
  SUBCASE("break and continue outside loop report binder error") {
    SymbolTableMgr::init();
    auto breakTree = SyntaxTree::Parse("break");
    auto breakBinder = std::make_unique<Binder>();
    breakBinder->BindCompilationUnit(breakTree->Root());
    REQUIRE_FALSE(breakBinder->Errors().empty());
    REQUIRE(breakBinder->Errors()[0].Message() ==
            "BinderError: The keyword break can only be used inside of loops.");

    auto contTree = SyntaxTree::Parse("continue");
    auto contBinder = std::make_unique<Binder>();
    contBinder->BindCompilationUnit(contTree->Root());
    REQUIRE_FALSE(contBinder->Errors().empty());
    REQUIRE(
        contBinder->Errors()[0].Message() ==
        "BinderError: The keyword continue can only be used inside of loops.");
  }
  SUBCASE("let immutability enforcement") {
    SymbolTableMgr::init();
    auto assignTree = SyntaxTree::Parse("let c = 10 c = 20");
    auto assignBinder = std::make_unique<Binder>();
    assignBinder->BindCompilationUnit(assignTree->Root());
    REQUIRE_FALSE(assignBinder->Errors().empty());
    REQUIRE(assignBinder->Errors()[0].Message() ==
            "BinderError: Variable c is read-only and cannot be assigned to.");

    auto compoundTree = SyntaxTree::Parse("let d = 5 { d += 1 }");
    auto compoundBinder = std::make_unique<Binder>();
    compoundBinder->BindCompilationUnit(compoundTree->Root());
    REQUIRE_FALSE(compoundBinder->Errors().empty());
    REQUIRE(compoundBinder->Errors()[0].Message() ==
            "BinderError: Variable d is read-only and cannot be assigned to.");
  }
}

TEST_CASE("SyntaxTree PrintTree") {
  SUBCASE("variable declarations let and var") {
    auto syntaxTree = SyntaxTree::Parse("let a = 10; var b = 20; a + b");
    REQUIRE(syntaxTree->Errors().empty());
    std::stringstream out;
    syntaxTree->PrintTree(out);
    std::string treeStr = out.str();
    REQUIRE(treeStr.find("VariableDeclaration") != std::string::npos);
    REQUIRE(treeStr.find("LetKeyword") != std::string::npos);
    REQUIRE(treeStr.find("VarKeyword") != std::string::npos);
  }
  SUBCASE("for loop without initializer") {
    auto syntaxTree = SyntaxTree::Parse("i = 0 for (; i < 3; i += 1) { i }");
    REQUIRE(syntaxTree->Errors().empty());
    std::stringstream out;
    syntaxTree->PrintTree(out);
    std::string treeStr = out.str();
    REQUIRE(treeStr.find("ForStatement") != std::string::npos);
  }
}
