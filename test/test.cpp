#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "Syntax/SyntaxTree.h"
#include "Binding/Binder.h"
#include "Evaluator.h"
#include <doctest/doctest.h>

Value testCaseHelper(std::string s) {
  auto syntaxTree = SyntaxTree::Parse(s);
  auto binder = std::make_unique<Binder>();
  auto boundExpression = binder->BindExpression(syntaxTree->Root());
  auto eval = std::make_unique<Evaluator>(std::move(boundExpression));
  return eval->Evaluate();
}

TEST_CASE("Binary Expression") {
  SUBCASE("negative numbers") {
    REQUIRE(-1 == testCaseHelper("-1").asInt());
  }
  SUBCASE("Simple Addition") {
    REQUIRE(4 == testCaseHelper("1 + 3").asInt());
  }
  SUBCASE("Simple Subtraction") {
    REQUIRE(1 == testCaseHelper("4-3").asInt());
  }
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
  SUBCASE("Simple Division") {
    REQUIRE(3 == testCaseHelper("9/3").asInt());
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
}