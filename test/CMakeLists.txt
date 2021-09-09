project(${CMAKE_PROJECT_NAME}_TEST)
cmake_minimum_required(VERSION 3.10)

download_file(https://raw.githubusercontent.com/onqtam/doctest/2.4.6/doctest/doctest.h 
              ${CMAKE_SOURCE_DIR}/packages/doctest/doctest.h
              MD5
              1494db8a28035bcf6caeea817c4b2b65)

include_directories(${CMAKE_SOURCE_DIR}/packages/)

add_executable(${PROJECT_NAME}
               #$<TARGET_OBJECTS:WarfLang.ParserTest>
               #$<TARGET_OBJECTS:WarfLang.LexerTest>
               #$<TARGET_OBJECTS:WarfLang.ASTTest>
               test.cpp
)

target_link_libraries (${PROJECT_NAME}
  WarfCore
)

#add_subdirectory(parser)
#add_subdirectory(lexer)
#add_subdirectory(ast)
