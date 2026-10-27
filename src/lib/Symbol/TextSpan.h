/*
 * Copyright (c) 2022 F. Lotfi All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#pragma once

#include <cstdint>
#include <string>

struct LineColumn {
  int32_t line;
  int32_t column;

  bool operator==(const LineColumn &other) const {
    return line == other.line && column == other.column;
  }
  bool operator!=(const LineColumn &other) const { return !(*this == other); }
};

class TextSpan {
public:
  using LineColumn = ::LineColumn;

  TextSpan() : mStart(0), mEnd(0) {}
  TextSpan(int start, int end);
  int32_t Start() const;
  int32_t Length() const;
  void updateTextSpan(int start, int end);
  int32_t End() const;
  int32_t EndPosition() const { return mEnd; }

  bool operator==(const TextSpan &other) const {
    return mStart == other.mStart && mEnd == other.mEnd;
  }
  bool operator!=(const TextSpan &other) const { return !(*this == other); }

  static LineColumn ComputeLineColumn(const std::string &sourceText,
                                      int32_t position);
  LineColumn StartLineColumn(const std::string &sourceText) const;
  LineColumn EndLineColumn(const std::string &sourceText) const;
  std::string FormatLocation(const std::string &sourceText) const;

private:
  int32_t mStart = 0;
  int32_t mEnd = 0;
};
