/*
 * Copyright (c) 2022 F. Lotfi All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#pragma once

#include <cstddef>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

#include "Binding/ValueType.h"
#include "Symbol/TextSpan.h"

#include "Syntax/SyntaxToken.h"

class Record {
public:
  Record(TextSpan span, std::string message);
  TextSpan Span() const;
  std::string Message() const;
  // bool operator==(const std::string &s) const;
private:
  TextSpan mSpan;
  std::string mMessage;
};

inline std::ostream &operator<<(std::ostream &out, const Record &r) {
  out << r.Message();
  return out;
}

class Records {
private:
  std::vector<Record> mRecords;
  std::string mPrefix;

public:
  Records(std::string prefix);
  Record &operator[](int index);
  const Record &operator[](int index) const;
  std::vector<Record>::iterator begin() { return mRecords.begin(); }
  std::vector<Record>::iterator end() { return mRecords.end(); }
  std::vector<Record>::const_iterator begin() const { return mRecords.begin(); }
  std::vector<Record>::const_iterator end() const { return mRecords.end(); }
  void assign(std::vector<Record>::const_iterator start,
              std::vector<Record>::const_iterator end);
  void swap(Records records);
  bool empty() const;
  size_t size() const;
  void Report(TextSpan span, std::string message);
  void ReportRangError(int32_t start, int32_t end, int64_t num, std::string overunder);
  void ReportOverflow(int32_t start, int32_t end, int64_t num)  {ReportRangError(start,end,num, "overflow" );}
  void ReportUnderflow(int32_t start, int32_t end, int64_t num) {ReportRangError(start,end,num, "underflow");}
  void ReportBadCharacter(int position, char character);
  void ReportUnexpectedToken(int32_t start, int32_t end, SyntaxType actual,
                             SyntaxType expected);
  void ReportUndefinedUnaryOperator(std::shared_ptr<SyntaxToken> unaryOperator,
                                    Type operandType);
  void
  ReportUndefinedBinaryOperator(std::shared_ptr<SyntaxToken> binaryOperator,
                                Type leftType, Type rightType);
  void ReportUndefinedIdentifier(std::shared_ptr<SyntaxToken> identifierToken);
};