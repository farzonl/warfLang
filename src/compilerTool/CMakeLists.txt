# Copyright (c) 2026 F. Lotfi All rights reserved.
# Use of this source code is governed by a BSD-style
# license that can be found in the LICENSE file.

project(warfc)

find_program(WARF_CLANG_EXECUTABLE NAMES clang)
if(NOT WARF_CLANG_EXECUTABLE)
  set(WARF_CLANG_EXECUTABLE clang)
endif()

add_executable(${PROJECT_NAME} main.cpp)

target_compile_definitions(${PROJECT_NAME} PRIVATE
  WARF_CLANG_EXECUTABLE="${WARF_CLANG_EXECUTABLE}"
)

target_link_libraries (${PROJECT_NAME}
  WarfCore
)
if(TARGET WarfLLVMBackend)
  target_link_libraries(${PROJECT_NAME} WarfLLVMBackend)
  target_compile_definitions(${PROJECT_NAME} PRIVATE WARF_ENABLE_LLVM_BACKEND=1)
endif()

target_include_directories(${PROJECT_NAME}
  PUBLIC ${CMAKE_SOURCE_DIR}/src/lib
         ${CMAKE_SOURCE_DIR}/src/llvmBackend
)