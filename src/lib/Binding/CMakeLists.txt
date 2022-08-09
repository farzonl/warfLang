include_directories(${WARF_CORE_SOURCE_DIR})

set(SourceFiles
    Binder.cpp
    BoundBinaryExpressionNode.cpp
    BoundLiteralExpressionNode.cpp
    BoundUnaryExpressionNode.cpp
    ValueType.cpp
    
)
set(HeaderFiles
    Binder.h
    BoundBinaryExpressionNode.h
    BoundExpressionNode.h
    BoundLiteralExpressionNode.h
    BoundUnaryExpressionNode.h
    ValueType.h
)

add_library (WarfCore.Binding OBJECT ${SourceFiles} ${HeaderFiles})