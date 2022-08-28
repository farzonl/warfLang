/*
 * Copyright (c) 2022 F. Lotfi All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#pragma once

#include <cstdint>

class TextSpan {
public:
  TextSpan(int start, int length);
  int32_t Start() const;
  int32_t Length() const;
  // TODO: delete this function when we have
  // Scoping better defined
  void SetLength(int32_t newLength);
  int32_t End() const;

private:
  int32_t mStart;
  int32_t mLength;
};