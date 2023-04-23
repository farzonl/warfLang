# Copyright (c) 2023 F. Lotfi All rights reserved.
# Use of this source code is governed by a BSD-style
# license that can be found in the LICENSE file.

include_directories(${WARF_CORE_SOURCE_DIR})

set(SourceFiles
    Scope.cpp
    SymbolTableMgr.cpp
    SymbolTable.cpp
    TextSpan.cpp
    VariableSymbol.cpp
    
    
)
set(HeaderFiles
    Scope.h
    SymbolTableMgr.h
    SymbolTable.h
    TextSpan.h
    VariableSymbol.h
)

add_library (WarfCore.Symbol OBJECT ${SourceFiles} ${HeaderFiles})