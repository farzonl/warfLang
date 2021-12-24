project(WarfCore)
cmake_minimum_required(VERSION 3.10)

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++latest /W4")
    # Default debug flags are OK 
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2")
else()
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g")
endif()

set(SourceFiles Lexer.cpp
                SyntaxToken.cpp
                SyntaxType.cpp
                SyntaxNode.cpp
                ExpressionNode.cpp
                LiteralExpression.cpp
                BinaryExpressionNode.cpp
                UnaryExpressionNode.cpp
                ParenthesizedExpressionNode.cpp
                Parser.cpp
                SyntaxTree.cpp
)
set(HeaderFiles Lexer.h
                SyntaxToken.h
                SyntaxType.h
                SyntaxNode.h
                ExpressionNode.h
                LiteralExpression.h
                BinaryExpressionNode.h
                UnaryExpressionNode.h
                ParenthesizedExpressionNode.h
                Parser.h
                SyntaxTree.h
)
add_library (${PROJECT_NAME} STATIC ${SourceFiles} ${HeaderFiles})