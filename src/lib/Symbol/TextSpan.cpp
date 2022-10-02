// Copyright (c) 2022 F. Lotfi All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "TextSpan.h"

TextSpan::TextSpan(int start, int end) : mStart(start), mEnd(end) {}

int32_t TextSpan::Start() const { return mStart; }

int32_t TextSpan::Length() const { return mEnd - mStart; }

int32_t TextSpan::End() const { return mEnd - 1; }

  void TextSpan::updateTextSpan(int start, int end) {
    mStart = start;
    mEnd = end;
  }