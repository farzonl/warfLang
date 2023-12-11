# Copyright (c) 2023 F. Lotfi All rights reserved.
# Use of this source code is governed by a BSD-style
# license that can be found in the LICENSE file.

include_directories(${WARF_CORE_SOURCE_DIR})

set(SourceFiles 
    Compilation.cpp
    Evaluator.cpp
)
set(HeaderFiles
    Compilation.h
    Evaluator.h
)

add_library (WarfCore.CodeAnalysis OBJECT ${SourceFiles} ${HeaderFiles})