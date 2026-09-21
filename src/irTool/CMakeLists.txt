# Copyright (c) 2026 F. Lotfi All rights reserved.
# Use of this source code is governed by a BSD-style
# license that can be found in the LICENSE file.

project(WarfIR)

find_program(WARF_LLVM_AS_EXECUTABLE NAMES llvm-as)
if(NOT WARF_LLVM_AS_EXECUTABLE)
  set(WARF_LLVM_AS_EXECUTABLE llvm-as)
endif()

add_executable(${PROJECT_NAME} main.cpp)

target_compile_definitions(${PROJECT_NAME} PRIVATE
  WARF_LLVM_AS_EXECUTABLE="${WARF_LLVM_AS_EXECUTABLE}"
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
