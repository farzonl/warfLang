// Copyright (c) 2022 F. Lotfi All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "Evaluator.h"
#include "Binding/BoundAssignmentExpressionNode.h"
#include "Binding/BoundBinaryExpressionNode.h"
#include "Binding/BoundBlockStatementNode.h"
#include "Binding/BoundBreakStatementNode.h"
#include "Binding/BoundCallExpressionNode.h"
#include "Binding/BoundContinueStatementNode.h"
#include "Binding/BoundDoWhileStatementNode.h"
#include "Binding/BoundExpressionStatementNode.h"
#include "Binding/BoundForStatementNode.h"
#include "Binding/BoundFunctionDeclarationNode.h"
#include "Binding/BoundIdentifierExpressionNode.h"
#include "Binding/BoundIfStatementNode.h"
#include "Binding/BoundLiteralExpressionNode.h"
#include "Binding/BoundReturnStatementNode.h"
#include "Binding/BoundUnaryExpressionNode.h"
#include "Binding/BoundVariableDeclarationNode.h"
#include "Binding/BoundWhileStatementNode.h"
#include <iostream>

BoundExpressionNode *Evaluator::Root() const {
  return dynamic_cast<BoundExpressionNode *>(mRoot.get());
}
Value Evaluator::Evaluate() {
  if (auto statement = dynamic_cast<BoundStatementNode *>(mRoot.get())) {
    return EvaluateStatement(statement);
  }
  return EvaluateRec(dynamic_cast<BoundExpressionNode *>(mRoot.get()));
}

Evaluator::Evaluator(std::unique_ptr<BoundExpressionNode> root)
    : mRoot(std::move(root)) {}

Evaluator::Evaluator(std::unique_ptr<BoundStatementNode> root)
    : mRoot(std::move(root)) {}

void Evaluator::SetVariableValue(VariableSymbol *variable, Value value) {
  if (!mCallFrames.empty()) {
    auto &frame = mCallFrames.back();
    if (frame.find(variable) == frame.end()) {
      frame[variable] = variable->GetValue();
    }
  }
  variable->SetValue(value);
}

Value Evaluator::EvaluateStatement(BoundStatementNode *node) {
  if (auto block = dynamic_cast<BoundBlockStatementNode *>(node)) {
    Value result;
    for (const auto &statement : block->Statements()) {
      result = EvaluateStatement(statement.get());
      if (mHasReturnValue) {
        return mReturnValue;
      }
      if (mShouldBreak || mShouldContinue) {
        break;
      }
    }
    return result;
  }
  if (dynamic_cast<BoundBreakStatementNode *>(node)) {
    mShouldBreak = true;
    return Value();
  }
  if (dynamic_cast<BoundContinueStatementNode *>(node)) {
    mShouldContinue = true;
    return Value();
  }
  if (auto returnStatement = dynamic_cast<BoundReturnStatementNode *>(node)) {
    Value val;
    if (returnStatement->Expression()) {
      val = EvaluateRec(
          const_cast<BoundExpressionNode *>(returnStatement->Expression()));
    }
    mReturnValue = val;
    mHasReturnValue = true;
    return val;
  }
  if (auto expression = dynamic_cast<BoundExpressionStatementNode *>(node)) {
    return EvaluateRec(
        const_cast<BoundExpressionNode *>(expression->Expression()));
  }
  if (auto declaration = dynamic_cast<BoundVariableDeclarationNode *>(node)) {
    auto value = EvaluateRec(
        const_cast<BoundExpressionNode *>(declaration->Initializer()));
    SetVariableValue(declaration->Variable().get(), value);
    return value;
  }
  if (auto declaration = dynamic_cast<BoundFunctionDeclarationNode *>(node)) {
    mFunctions[declaration->Function()->Name()] = declaration;
    return Value();
  }
  if (auto ifStatement = dynamic_cast<BoundIfStatementNode *>(node)) {
    auto condition = EvaluateRec(
        const_cast<BoundExpressionNode *>(ifStatement->Condition()));
    if (condition.asBool()) {
      return EvaluateStatement(
          const_cast<BoundStatementNode *>(ifStatement->ThenStatement()));
    }
    if (ifStatement->ElseStatement()) {
      return EvaluateStatement(
          const_cast<BoundStatementNode *>(ifStatement->ElseStatement()));
    }
    return Value();
  }
  if (auto whileStatement = dynamic_cast<BoundWhileStatementNode *>(node)) {
    Value result;
    while (EvaluateRec(
               const_cast<BoundExpressionNode *>(whileStatement->Condition()))
               .asBool()) {
      result = EvaluateStatement(
          const_cast<BoundStatementNode *>(whileStatement->Body()));
      if (mHasReturnValue) {
        return mReturnValue;
      }
      if (mShouldBreak) {
        mShouldBreak = false;
        break;
      }
      if (mShouldContinue) {
        mShouldContinue = false;
        continue;
      }
    }
    return result;
  }
  if (auto doWhileStatement = dynamic_cast<BoundDoWhileStatementNode *>(node)) {
    Value result;
    do {
      result = EvaluateStatement(
          const_cast<BoundStatementNode *>(doWhileStatement->Body()));
      if (mHasReturnValue) {
        return mReturnValue;
      }
      if (mShouldBreak) {
        mShouldBreak = false;
        break;
      }
      if (mShouldContinue) {
        mShouldContinue = false;
      }
    } while (EvaluateRec(const_cast<BoundExpressionNode *>(
                             doWhileStatement->Condition()))
                 .asBool());
    return result;
  }
  if (auto forStatement = dynamic_cast<BoundForStatementNode *>(node)) {
    Value result;
    if (forStatement->Initializer()) {
      EvaluateStatement(
          const_cast<BoundStatementNode *>(forStatement->Initializer()));
    }
    while (EvaluateRec(
               const_cast<BoundExpressionNode *>(forStatement->Condition()))
               .asBool()) {
      result = EvaluateStatement(
          const_cast<BoundStatementNode *>(forStatement->Body()));
      if (mHasReturnValue) {
        return mReturnValue;
      }
      if (mShouldBreak) {
        mShouldBreak = false;
        break;
      }
      if (mShouldContinue) {
        mShouldContinue = false;
      }
      EvaluateRec(const_cast<BoundExpressionNode *>(forStatement->Increment()));
    }
    return result;
  }
  throw std::runtime_error("EvaluatorError: Unexpected statement");
}

Value Evaluator::EvaluateRec(BoundExpressionNode *node) {
  if (BoundLiteralExpressionNode *literal =
          dynamic_cast<BoundLiteralExpressionNode *>(node)) {
    return literal->GetValue();
  }
  if (BoundIdentifierExpressionNode *identifierExpression =
          dynamic_cast<BoundIdentifierExpressionNode *>(node)) {
    return identifierExpression->Variable()->GetValue();
  }
  if (BoundAssignmentExpressionNode *assignmentExpression =
          dynamic_cast<BoundAssignmentExpressionNode *>(node)) {
    auto rightSide = EvaluateRec(assignmentExpression->BoundExpression());
    // TODO add assignment operator types
    auto opKind = assignmentExpression->OperatorKind();
    Value returnValue;
    switch (opKind) {
    case BoundAssignmentOperatorKind::Assignment:
      returnValue = rightSide;
      break;
    case BoundAssignmentOperatorKind::AddAndAssign:
      returnValue = assignmentExpression->Variable()->GetValue() + rightSide;
      break;
    case BoundAssignmentOperatorKind::SubtractAndAssign:
      returnValue = assignmentExpression->Variable()->GetValue() - rightSide;
      break;
    case BoundAssignmentOperatorKind::MultiplyAndAssign:
      returnValue = assignmentExpression->Variable()->GetValue() * rightSide;
      break;
    case BoundAssignmentOperatorKind::DivideAndAssign:
      returnValue = assignmentExpression->Variable()->GetValue() / rightSide;
      break;
    case BoundAssignmentOperatorKind::BitwiseAndAndAssign:
      returnValue = assignmentExpression->Variable()->GetValue() & rightSide;
      break;
    case BoundAssignmentOperatorKind::BitwiseOrAndAssign:
      returnValue = assignmentExpression->Variable()->GetValue() | rightSide;
      break;
    case BoundAssignmentOperatorKind::BitwiseXorAndAssign:
      returnValue = assignmentExpression->Variable()->GetValue() ^ rightSide;
      break;
    }
    SetVariableValue(assignmentExpression->Variable().get(), returnValue);
    return returnValue;
  }
  if (BoundUnaryExpressionNode *unaryExpression =
          dynamic_cast<BoundUnaryExpressionNode *>(node)) {
    auto opKind = unaryExpression->OperatorKind();
    auto operand = EvaluateRec(unaryExpression->Operand());
    switch (opKind) {
    case BoundUnaryOperatorKind::Identity:
      return operand;
    case BoundUnaryOperatorKind::Negation:
      return -operand;
    case BoundUnaryOperatorKind::LogicalNegation:
      return !operand;
    case BoundUnaryOperatorKind::BitwiseNot:
      return ~operand;
    default:
      throw std::runtime_error("EvaluatorError: Unexpected unary operator: " +
                               BoundUnaryKindStrMap.at(opKind));
    }
  }
  if (BoundBinaryExpressionNode *binaryExpression =
          dynamic_cast<BoundBinaryExpressionNode *>(node)) {
    auto left = EvaluateRec(binaryExpression->Left());
    auto opKind = binaryExpression->OperatorKind();
    if (opKind == BoundBinaryOperatorKind::LogicalAnd) {
      if (!left.asBool()) {
        return left;
      }
      auto right = EvaluateRec(binaryExpression->Right());
      return left && right;
    }
    if (opKind == BoundBinaryOperatorKind::LogicalOr) {
      if (left.asBool()) {
        return left;
      }
      auto right = EvaluateRec(binaryExpression->Right());
      return left || right;
    }
    auto right = EvaluateRec(binaryExpression->Right());
    switch (opKind) {
    case BoundBinaryOperatorKind::Addition:
      return left + right;
    case BoundBinaryOperatorKind::Subtraction:
      return left - right;
    case BoundBinaryOperatorKind::Multiplication:
      return left * right;
    case BoundBinaryOperatorKind::Division:
      return left / right;
    case BoundBinaryOperatorKind::Equals:
      return left == right;
    case BoundBinaryOperatorKind::NotEquals:
      return left != right;
    case BoundBinaryOperatorKind::LogicalAnd:
      return left && right;
    case BoundBinaryOperatorKind::LogicalOr:
      return left || right;
    case BoundBinaryOperatorKind::BitwiseAnd:
      return left & right;
    case BoundBinaryOperatorKind::BitwiseOr:
      return left | right;
    case BoundBinaryOperatorKind::BitwiseXor:
      return left ^ right;
    case BoundBinaryOperatorKind::GreaterThan:
      return left > right;
    case BoundBinaryOperatorKind::GreaterThanOrEqualTo:
      return left >= right;
    case BoundBinaryOperatorKind::LessThan:
      return left < right;
    case BoundBinaryOperatorKind::LessThanOrEqualTo:
      return left <= right;
    default:
      throw std::runtime_error("EvaluatorError: Unexpected binary operator: " +
                               BoundBinaryKindStrMap.at(opKind));
      return Value();
    }
  }
  if (BoundCallExpressionNode *callExpression =
          dynamic_cast<BoundCallExpressionNode *>(node)) {
    auto function = mFunctions.find(callExpression->Name());
    if (function == mFunctions.end()) {
      if (callExpression->Name() == "print") {
        auto val = EvaluateRec(callExpression->Arguments()[0].get());
        std::cout << val << std::endl;
        return val;
      }
      throw std::runtime_error("EvaluatorError: Unexpected function: " +
                               callExpression->Name());
    }
    std::vector<Value> argValues;
    argValues.reserve(callExpression->Arguments().size());
    for (const auto &arg : callExpression->Arguments()) {
      argValues.push_back(EvaluateRec(arg.get()));
    }

    const BoundCallExpressionNode *targetCall = callExpression;
    if (callExpression->Body()) {
      mActiveCallNodes[callExpression->Name()] = callExpression;
    } else {
      targetCall = mActiveCallNodes.at(callExpression->Name());
    }

    mCallFrames.emplace_back();
    const auto &parameters = targetCall->Parameters();
    for (size_t i = 0; i < parameters.size(); i++) {
      SetVariableValue(parameters[i].get(), argValues[i]);
    }

    bool prevHasReturn = mHasReturnValue;
    bool prevShouldBreak = mShouldBreak;
    bool prevShouldContinue = mShouldContinue;
    Value prevReturnVal = mReturnValue;
    mHasReturnValue = false;
    mShouldBreak = false;
    mShouldContinue = false;

    Value result =
        EvaluateStatement(const_cast<BoundStatementNode *>(targetCall->Body()));
    if (mHasReturnValue) {
      result = mReturnValue;
    }

    mHasReturnValue = prevHasReturn;
    mShouldBreak = prevShouldBreak;
    mShouldContinue = prevShouldContinue;
    mReturnValue = prevReturnVal;

    for (const auto &[var, savedVal] : mCallFrames.back()) {
      var->SetValue(savedVal);
    }
    mCallFrames.pop_back();

    return result;
  }
  return Value();
}
