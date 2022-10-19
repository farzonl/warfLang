include_directories(${WARF_CORE_SOURCE_DIR})

set(SourceFiles
    Binder.cpp
    BoundBinaryExpressionNode.cpp
    BoundLiteralExpressionNode.cpp
    BoundUnaryExpressionNode.cpp
    BoundAssignmentExpressionNode.cpp
    BoundIdentifierExpressionNode.cpp
    BoundStatement.cpp
    BoundBlockStatement.cpp
    BoundExpressionStatement.cpp
    ValueType.cpp
    
)
set(HeaderFiles
    Binder.h
    BoundBinaryExpressionNode.h
    BoundExpressionNode.h
    BoundLiteralExpressionNode.h
    BoundUnaryExpressionNode.h
    BoundAssignmentExpressionNode.h
    BoundIdentifierExpressionNode.h
    BoundStatement.h
    BoundBlockStatement.h
    BoundExpressionStatement.h
    ValueType.h
)

add_library (WarfCore.Binding OBJECT ${SourceFiles} ${HeaderFiles})