#include "BoundBlockStatement.h"
#include <iterator>

const std::vector<std::unique_ptr<BoundStatement>> &
BoundBlockStatement::Statements() const {
  return mStatements;
}

BoundBlockStatement::BoundBlockStatement(
    std::vector<std::unique_ptr<BoundStatement>> &statements)
    : BoundStatement(), mStatements() {
  std::move(std::begin(statements), std::end(statements),
            std::back_inserter(mStatements));
}

BoundNodeType BoundBlockStatement::NodeType() {
  return BoundNodeType::BlockStatement;
}