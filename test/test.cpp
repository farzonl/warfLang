// Copyright (c) 2022 F. Lotfi All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "Binding/Binder.h"
#include "Evaluator.h"
#include "Symbol/SymbolTableMgr.h"
#include "Syntax/SyntaxTree.h"
#include <doctest/doctest.h>

Value testCaseHelper(std::string s) {
  SymbolTableMgr::init();
  auto syntaxTree = SyntaxTree::Parse(s);
  auto binder = std::make_unique<Binder>();
  auto boundExpression =
      binder->BindExpression(syntaxTree->Root()->Expression());
  auto eval = std::make_unique<Evaluator>(std::move(boundExpression));
  return eval->Evaluate();
}

bool testCaseSyntaxErrors(std::string s, std::string errorStr) {
  SymbolTableMgr::init();
  auto syntaxTree = SyntaxTree::Parse(s);
  if (!syntaxTree->Errors().empty()) {
    return errorStr == syntaxTree->Errors()[0].Message();
  }
  auto binder = std::make_unique<Binder>();
  auto boundExpression =
      binder->BindExpression(syntaxTree->Root()->Expression());
  if (!binder->Errors().empty()) {
    return errorStr == binder->Errors()[0].Message();
  }
  try {
    auto eval = std::make_unique<Evaluator>(std::move(boundExpression));
    eval->Evaluate();
  } catch (std::runtime_error &error) {
    return errorStr == error.what();
  }
  return false;
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