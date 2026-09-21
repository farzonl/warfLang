# Copyright (c) 2026 F. Lotfi All rights reserved.
# Use of this source code is governed by a BSD-style
# license that can be found in the LICENSE file.

project(WarfLLVMBackend)

add_library(${PROJECT_NAME} STATIC
  LlvmBackend.cpp
  LlvmBackend.h
)

target_include_directories(${PROJECT_NAME}
  PUBLIC ${CMAKE_SOURCE_DIR}/src/lib
  PRIVATE ${LLVM_INCLUDE_DIRS}
)
target_compile_definitions(${PROJECT_NAME} PRIVATE ${LLVM_DEFINITIONS})

llvm_map_components_to_libnames(WARF_LLVM_LIBS
  bitwriter
  core
  support
)

target_link_libraries(${PROJECT_NAME}
  PUBLIC WarfCore
  PRIVATE ${WARF_LLVM_LIBS}
)