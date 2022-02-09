include_directories(${WARF_CORE_SOURCE_DIR})

set(SourceFiles 
    ValueType.cpp
)
set(HeaderFiles 
    ValueType.h
)

add_library (WarfCore.Binding OBJECT ${SourceFiles} ${HeaderFiles})