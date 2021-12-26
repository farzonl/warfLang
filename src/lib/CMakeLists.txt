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

set(SourceFiles Syntax/Lexer.cpp
                Syntax/SyntaxToken.cpp
                Syntax/SyntaxType.cpp
                Syntax/SyntaxNode.cpp
                Syntax/ExpressionNode.cpp
                Syntax/LiteralExpressionNode.cpp
                Syntax/BinaryExpressionNode.cpp
                Syntax/UnaryExpressionNode.cpp
                Syntax/ParenthesizedExpressionNode.cpp
                Syntax/Parser.cpp
                Syntax/SyntaxTree.cpp
)
set(HeaderFiles Syntax/Lexer.h
                Syntax/SyntaxToken.h
                Syntax/SyntaxType.h
                Syntax/SyntaxNode.h
                Syntax/ExpressionNode.h
                Syntax/LiteralExpressionNode.h
                Syntax/BinaryExpressionNode.h
                Syntax/UnaryExpressionNode.h
                Syntax/ParenthesizedExpressionNode.h
                Syntax/Parser.h
                Syntax/SyntaxTree.h
)
add_library (${PROJECT_NAME} STATIC ${SourceFiles} ${HeaderFiles})