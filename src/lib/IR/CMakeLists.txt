# Copyright (c) 2026 F. Lotfi All rights reserved.
# Use of this source code is governed by a BSD-style
# license that can be found in the LICENSE file.

include_directories(${WARF_CORE_SOURCE_DIR})

set(SourceFiles
    Ir.cpp
)
set(HeaderFiles
    Ir.h
)

add_library (WarfCore.IR OBJECT ${SourceFiles} ${HeaderFiles})
target_precompile_headers(WarfCore.IR REUSE_FROM WarfCore.Syntax)