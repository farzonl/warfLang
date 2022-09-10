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
  //bool operator==(const std::string &s) const;
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
  struct Iterator {
    using iterator_category = std::forward_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = Record;
    using pointer = Record *;
    using reference = Record &;

    Iterator(pointer ptr) : m_ptr(ptr) {}

    reference operator*() const { return *m_ptr; }
    pointer operator->() { return m_ptr; }
    Iterator &operator++() {
      m_ptr++;
      return *this;
    }
    Iterator operator++(int) {
      Iterator tmp = *this;
      ++(*this);
      return tmp;
    }
    friend bool operator==(const Iterator &a, const Iterator &b) {
      return a.m_ptr == b.m_ptr;
    };
    friend bool operator!=(const Iterator &a, const Iterator &b) {
      return a.m_ptr != b.m_ptr;
    };

  private:
    pointer m_ptr;
  };
  Record &operator[](int index);
  const Record &operator[](int index) const;
  Iterator begin() { return Iterator(&mRecords[0]); }
  Iterator end() { return Iterator(&mRecords[mRecords.size() - 1]); }
  const Iterator begin() const { return Iterator(const_cast<Record*>(&mRecords[0])); }
  const Iterator end() const { return Iterator(const_cast<Record*>(&mRecords[mRecords.size() - 1])); }
  void assign(const Iterator start, const Iterator end);
  void swap(Records records);
  bool empty() const;
  size_t size() const;
  void Report(TextSpan span, std::string message);
  void ReportOverflow(int32_t start, int32_t end, int64_t num);
  void ReportUnderflow(int32_t start, int32_t end, int64_t num);
  void ReportBadCharacter(int position, char character);
  void ReportUnexpectedToken(int32_t start, int32_t end, SyntaxType actual,
                             SyntaxType expected);
  void ReportUndefinedUnaryOperator(std::shared_ptr<SyntaxToken> unaryOperator,
                                    Type operandType);
  void ReportUndefinedBinaryOperator(std::shared_ptr<SyntaxToken> binaryOperator, Type leftType,
                                     Type rightType);
  void ReportUndefinedIdentifier(std::shared_ptr<SyntaxToken> identifierToken);
};