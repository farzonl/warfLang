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