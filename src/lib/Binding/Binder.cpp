
#include "Binder.h"
#include "BoundLiteralExpressionNode.h"
#include "BoundUnaryExpressionNode.h"
#include "Syntax/BinaryExpressionNode.h"
#include "Syntax/LiteralExpressionNode.h"
#include "Syntax/UnaryExpressionNode.h"

std::unique_ptr<BoundExpressionNode>
Binder::BindExpression(ExpressionNode *node) {

  if (LiteralExpressionNode *literal =
          dynamic_cast<LiteralExpressionNode *>(node)) {
    return std::move(BindLiteralExpression(literal));
  }
  if (UnaryExpressionNode *unaryExpression =
          dynamic_cast<UnaryExpressionNode *>(node)) {
    return std::move(BindUnaryExpression(unaryExpression));
  }
  if (BinaryExpressionNode *binaryExpression =
          dynamic_cast<BinaryExpressionNode *>(node)) {
    return std::move(BindBinaryExpression(binaryExpression));
  }

  throw "Unexpected syntax ";
  return nullptr;
}

std::unique_ptr<BoundExpressionNode>
Binder::BindLiteralExpression(LiteralExpressionNode *literal) {
  std::make_unique<BoundLiteralExpressionNode>(literal);
}

std::unique_ptr<BoundExpressionNode>
Binder::BindUnaryExpression(UnaryExpressionNode *unary) {
  auto boundOperand = BindExpression(unary->Operand());
  auto boundOperator = BoundUnaryOperator::Bind(unary->Operator()->Type(),
                                                boundOperand->GetType());
  if (&boundOperator == &BoundUnaryOperator::GetBindFailure()) {
    // TODO
    std::cerr << "Unary operator " << unary->Operator()
              << " is not defined for type " << boundOperand->GetType() << ".";
    return boundOperand;
  }
  return std::make_unique<BoundExpressionNode>(boundOperator, boundOperand);
}

std::unique_ptr<BoundExpressionNode>
Binder::BindBinaryExpression(BinaryExpressionNode *binary) {}