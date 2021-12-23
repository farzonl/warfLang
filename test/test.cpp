#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>
#include "SyntaxTree.h"

TEST_CASE("Binary Expression") {
    SUBCASE("Simple Addition") {
        auto syntaxTree = SyntaxTree::Parse("1+3");
        REQUIRE(4 == syntaxTree->Evaluate());
    }
    SUBCASE("Simple Subtraction") {
        auto syntaxTree = SyntaxTree::Parse("4-3");
        REQUIRE(1 == syntaxTree->Evaluate());
    }
    SUBCASE("Simple Negative Number") {
        auto syntaxTree = SyntaxTree::Parse("3-4");
        REQUIRE(-1 == syntaxTree->Evaluate());
    }
    SUBCASE("Simple Multiplication") {
        auto syntaxTree = SyntaxTree::Parse("4*3");
        REQUIRE(12 == syntaxTree->Evaluate());
    }
    //SUBCASE(" Multiplication of Negative") {
    //    auto syntaxTree = SyntaxTree::Parse("4*-5");
    //    REQUIRE(-20 == syntaxTree->Evaluate());
    //}
    //SUBCASE(" Multiplication of Negative") {
    //    auto syntaxTree = SyntaxTree::Parse("4*(-5)");
    //    REQUIRE(-20 == syntaxTree->Evaluate());
    //}
    SUBCASE(" Multiplication of Negative") {
        auto syntaxTree = SyntaxTree::Parse("4*(3-5)");
        REQUIRE(-8 == syntaxTree->Evaluate());
    }
    SUBCASE("Simple Division") {
        auto syntaxTree = SyntaxTree::Parse("9/3");
        REQUIRE(3 == syntaxTree->Evaluate());
    }
    SUBCASE("Simple PMDAS Ordering") {
        auto syntaxTree = SyntaxTree::Parse("4*1+3");
        REQUIRE(7 == syntaxTree->Evaluate());
    }
    SUBCASE("Simple Parentheses") {
        auto syntaxTree = SyntaxTree::Parse("4*(1+3)");
        REQUIRE(16 == syntaxTree->Evaluate());
    }
}