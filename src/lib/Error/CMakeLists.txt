include_directories(${WARF_CORE_SOURCE_DIR})

set(SourceFiles
    Record.cpp
)
set(HeaderFiles
    Record.h
)

add_library (WarfCore.Error OBJECT ${SourceFiles} ${HeaderFiles})