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
  /*SUBCASE("Multiplication of Negative Parentheses") {
    REQUIRE(-20 == testCaseHelper("4*(-5)").asInt());
  }
  SUBCASE("Multiplication of Negative Created in Parentheses") {
    REQUIRE(-8 == testCaseHelper("4*(3-5)").asInt());
  }*/
  SUBCASE("Simple Division") {
    REQUIRE(3 == testCaseHelper("9/3").asInt());
  }
  SUBCASE("Simple PMDAS Ordering") {
    REQUIRE(7 == testCaseHelper("4*1+3").asInt());
  }
  /*SUBCASE("Simple Parentheses") {
    REQUIRE(16 == testCaseHelper("4*(1+3)").asInt());
  }
  SUBCASE("Negative Parentheses") {
    REQUIRE(-4 == testCaseHelper("-(1+3)").asInt());
  }*/
}