/*
 * Copyright (c) 2026 F. Lotfi All rights reserved.
 * Use of this source code is governed by a BSD-style
 * license that can be found in the LICENSE file.
 */

// Precompiled header: standard library includes shared by every WarfCore
// target. Missing one of these in an individual header is a common source
// of MSVC/GCC-only build failures (e.g. std::unique_ptr without <memory>);
// listing them here means every translation unit in the project gets them
// for free instead of relying on transitive includes.
#pragma once

#include <cassert>
#include <functional>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>
