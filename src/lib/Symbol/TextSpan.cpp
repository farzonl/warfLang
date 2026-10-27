// Copyright (c) 2022 F. Lotfi All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "TextSpan.h"

TextSpan::TextSpan(int start, int end) : mStart(start), mEnd(end) {}

int32_t TextSpan::Start() const { return mStart; }

int32_t TextSpan::Length() const { return mEnd - mStart; }

void TextSpan::updateTextSpan(int start, int end) {
  mStart = start;
  mEnd = end;
}

int32_t TextSpan::End() const { return mEnd - 1; }

LineColumn TextSpan::ComputeLineColumn(const std::string &sourceText,
                                       int32_t position) {
  int32_t line = 1;
  int32_t column = 1;
  if (position <= 0 || sourceText.empty()) {
    return {line, column};
  }
  int32_t limit = static_cast<int32_t>(sourceText.size());
  if (position < limit) {
    limit = position;
  }
  for (int32_t i = 0; i < limit; ++i) {
    if (sourceText[i] == '\n') {
      ++line;
      column = 1;
    } else if (sourceText[i] != '\r') {
      ++column;
    }
  }
  return {line, column};
}

LineColumn TextSpan::StartLineColumn(const std::string &sourceText) const {
  return ComputeLineColumn(sourceText, Start());
}

LineColumn TextSpan::EndLineColumn(const std::string &sourceText) const {
  int32_t endPos = End();
  if (endPos < Start()) {
    endPos = Start();
  }
  return ComputeLineColumn(sourceText, endPos);
}

std::string TextSpan::FormatLocation(const std::string &sourceText) const {
  LineColumn loc = StartLineColumn(sourceText);
  return std::to_string(loc.line) + ":" + std::to_string(loc.column);
}
