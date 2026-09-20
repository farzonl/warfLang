# Copyright (c) 2023 F. Lotfi All rights reserved.
# Use of this source code is governed by a BSD-style
# license that can be found in the LICENSE file.

include_directories(${WARF_CORE_SOURCE_DIR})

set(SourceFiles
    Binder.cpp
    BoundBinaryExpressionNode.cpp
    BoundCallExpressionNode.cpp
    BoundLiteralExpressionNode.cpp
    BoundUnaryExpressionNode.cpp
    BoundAssignmentExpressionNode.cpp
    BoundIdentifierExpressionNode.cpp
    BoundExpressionStatementNode.cpp
    BoundBlockStatementNode.cpp
    BoundVariableDeclarationNode.cpp
    BoundFunctionDeclarationNode.cpp
    BoundIfStatementNode.cpp
    BoundWhileStatementNode.cpp
    BoundForStatementNode.cpp
    ValueType.cpp
    
)
set(HeaderFiles
    Binder.h
    BoundBinaryExpressionNode.h
    BoundCallExpressionNode.h
    BoundExpressionNode.h
    BoundExpressionStatementNode.h
    BoundBlockStatementNode.h
    BoundLiteralExpressionNode.h
    BoundUnaryExpressionNode.h
    BoundAssignmentExpressionNode.h
    BoundIdentifierExpressionNode.h
    BoundVariableDeclarationNode.h
    BoundFunctionDeclarationNode.h
    BoundIfStatementNode.h
    BoundWhileStatementNode.h
    BoundForStatementNode.h
    ValueType.h
)

add_library (WarfCore.Binding OBJECT ${SourceFiles} ${HeaderFiles})
target_precompile_headers(WarfCore.Binding REUSE_FROM WarfCore.Syntax)
