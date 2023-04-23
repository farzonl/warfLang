# Copyright (c) 2023 F. Lotfi All rights reserved.
# Use of this source code is governed by a BSD-style
# license that can be found in the LICENSE file.

include_directories(${WARF_CORE_SOURCE_DIR})

set(SourceFiles
    Record.cpp
)
set(HeaderFiles
    Record.h
)

add_library (WarfCore.Error OBJECT ${SourceFiles} ${HeaderFiles})