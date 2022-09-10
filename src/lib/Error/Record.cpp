#include "Record.h"
#include "Syntax/SyntaxToken.h"
#include <sstream>

Records::Records(std::string prefix) : mPrefix(prefix) {}

void Records::Report(TextSpan span, std::string message) {
  mRecords.push_back(Record(span, message));
}

void Records::assign(const Iterator start, const Iterator end) {
  mRecords.assign(start, end);
}

void Records::swap(Records records) { mRecords.swap(records.mRecords); }

bool Records::empty() const { return mRecords.empty(); }

size_t Records::size() const { return mRecords.size(); }

void Records::ReportOverflow(int32_t start, int32_t end, int64_t num) {
  std::stringstream message;
  message << mPrefix << "Error: The number " << num
          << "is a Numeric underflow.";
  Report(TextSpan(start, end), message.str());
}

void Records::ReportUnderflow(int32_t start, int32_t end, int64_t num) {
  std::stringstream message;
  message << mPrefix << "Error: The number " << num << "is a Numeric overflow.";
  Report(TextSpan(start, end), message.str());
}

void Records::ReportBadCharacter(int position, char character) {
  std::stringstream message;
  message << mPrefix << "Error: Bad character input: " << character << '.';
  Report(TextSpan(position, 1), message.str());
}

void Records::ReportUnexpectedToken(int32_t start, int32_t end,
                                    SyntaxType actual, SyntaxType expected) {

  std::stringstream message;
  message << mPrefix << "Error: Unexpected token: < " << actual
          << " > Expected: < " << expected << " >";
  Report(TextSpan(start, end), message.str());
}

void Records::ReportUndefinedUnaryOperator(SyntaxToken *unaryOperator,
                                           Type operandType) {

  std::stringstream message;
  message << "Error: Unary operator "
          << SyntaxTokenToStrMap.at(unaryOperator->Type())
          << " is not defined for type " << operandType << ".";
  Report(unaryOperator->Span(), message.str());
}

/*void Records::ReportUndefinedUnaryOperatorEvaluation(int32_t start, int32_t
end, BoundUnaryOperatorType operatorType, Type valueType) { std::stringstream
message; message << mPrefix << "Error: Unary operator " <<
BoundUnaryTypeStrMap.at(operatorType)
         << "is not defined for type " << valueType
         << ".";
}*/

void Records::ReportUndefinedBinaryOperator(SyntaxToken *binaryOperator,
                                            Type leftType, Type rightType) {
  std::stringstream message;
  message << "Error: Binary operator "
          << SyntaxTokenToStrMap.at(binaryOperator->Type())
          << " is not defined for types " << leftType << " and " << rightType
          << "." << std::endl;
  Report(binaryOperator->Span(), message.str());
}

void Records::ReportUndefinedIdentifier(
    std::shared_ptr<SyntaxToken> identifierToken) {
  std::stringstream message;
  message << "Undefined name: " << identifierToken->Text()
          << "Starting at Position " << identifierToken->Span().Start()
          << " Ending at: " << identifierToken->Span().End() << "."
          << std::endl;
  Report(identifierToken->Span(), message.str());
}