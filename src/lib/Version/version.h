/*
 * Copyright (c) 2021 Farzon Lotfi All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

#ifndef __version_h__
#define __version_h__

#include <string>

namespace WarfHelper {
struct Version {
  static const std::string number;
  static const std::string name;

private:
  Version() = delete;
};
struct Git {
  static const std::string sha1;
  static const std::string date;
  static const std::string subject;
  static const std::string branch;

private:
  Git() = delete;
};

void printVersion();

} // namespace WarfHelper

#endif //__version_h__