// Copyright (c) 2022 F. Lotfi All rights reserved.
// Use of this source code is governed by a BSD-style
// license that can be found in the LICENSE file.

#include "TextSpan.h"

TextSpan::TextSpan(int start, int length) : mStart(start), mLength(length) {}

int32_t TextSpan::Start() const { return mStart; }

int32_t TextSpan::Length() const { return mLength; }

void TextSpan::SetLength(int32_t newLength) { mLength = newLength; }

int32_t TextSpan::End() const { return mStart + mLength; }