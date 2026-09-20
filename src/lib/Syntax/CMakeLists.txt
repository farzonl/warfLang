# Copyright (c) 2023 F. Lotfi All rights reserved.
# Use of this source code is governed by a BSD-style
# license that can be found in the LICENSE file.

include_directories(${WARF_CORE_SOURCE_DIR})

set(SourceFiles Lexer.cpp
                SyntaxToken.cpp
                SyntaxKind.cpp
                SyntaxNode.cpp
                BlockStatementSyntaxNode.cpp
                CompilationUnitSyntaxNode.cpp
                StatementSyntaxNode.cpp
                ExpressionStatementSyntaxNode.cpp
                VariableDeclarationSyntaxNode.cpp
                FunctionDeclarationSyntaxNode.cpp
                IfStatementSyntaxNode.cpp
                WhileStatementSyntaxNode.cpp
                ForStatementSyntaxNode.cpp
                ExpressionNode.cpp
                AssignmentExpressionNode.cpp
                CallExpressionNode.cpp
                LiteralExpressionNode.cpp
                BinaryExpressionNode.cpp
                UnaryExpressionNode.cpp
                ParenthesizedExpressionNode.cpp
                IdentifierExpressionNode.cpp
                Parser.cpp
                SyntaxTree.cpp
)
set(HeaderFiles Lexer.h
                SyntaxToken.h
                SyntaxKind.h
                SyntaxNode.h
                BlockStatementSyntaxNode.h
                CompilationUnitSyntaxNode.h 
                StatementSyntaxNode.h
                ExpressionStatementSyntaxNode.h
                VariableDeclarationSyntaxNode.h
                FunctionDeclarationSyntaxNode.h
                IfStatementSyntaxNode.h
                WhileStatementSyntaxNode.h
                ForStatementSyntaxNode.h
                ExpressionNode.h
                AssignmentExpressionNode.h
                CallExpressionNode.h
                LiteralExpressionNode.h
                BinaryExpressionNode.h
                UnaryExpressionNode.h
                ParenthesizedExpressionNode.h
                IdentifierExpressionNode.h
                Parser.h
                SyntaxTree.h
)
add_library (WarfCore.Syntax OBJECT ${SourceFiles} ${HeaderFiles})
target_precompile_headers(WarfCore.Syntax PRIVATE ${WARF_CORE_SOURCE_DIR}/CommonPCH.h)