include_directories(${WARF_CORE_SOURCE_DIR})

set(SourceFiles CompilationUnitNode.cpp
                Lexer.cpp
                SyntaxToken.cpp
                SyntaxType.cpp
                SyntaxNode.cpp
                ExpressionNode.cpp
                AssignmentExpressionNode.cpp
                LiteralExpressionNode.cpp
                BinaryExpressionNode.cpp
                UnaryExpressionNode.cpp
                ParenthesizedExpressionNode.cpp
                IdentifierExpressionNode.cpp
                Parser.cpp
                SyntaxTree.cpp
)
set(HeaderFiles CompilationUnitNode.h
                Lexer.h
                SyntaxToken.h
                SyntaxType.h
                SyntaxNode.h
                ExpressionNode.h
                AssignmentExpressionNode.h
                LiteralExpressionNode.h
                BinaryExpressionNode.h
                UnaryExpressionNode.h
                ParenthesizedExpressionNode.h
                IdentifierExpressionNode.h
                Parser.h
                SyntaxTree.h
)
add_library (WarfCore.Syntax OBJECT ${SourceFiles} ${HeaderFiles})