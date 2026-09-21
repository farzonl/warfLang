# Copyright (c) 2023 F. Lotfi All rights reserved.
# Use of this source code is governed by a BSD-style
# license that can be found in the LICENSE file.

add_subdirectory(lib)
if(NOT isEmscripten AND NOT CMAKE_CROSSCOMPILING AND NOT CMAKE_CROSS_COMPILING)
    find_package(LLVM CONFIG QUIET)
    if(NOT LLVM_FOUND)
        file(GLOB WARF_HOMEBREW_LLVM_CONFIGS
            /opt/homebrew/Cellar/llvm/*/lib/cmake/llvm
            /usr/local/opt/llvm/lib/cmake/llvm
        )
        if(WARF_HOMEBREW_LLVM_CONFIGS)
            find_package(LLVM CONFIG QUIET PATHS ${WARF_HOMEBREW_LLVM_CONFIGS})
        endif()
    endif()
    if(LLVM_FOUND)
        add_subdirectory(llvmBackend)
    endif()
endif()
if(isEmscripten)
    add_subdirectory(wasmEntry)
endif()
add_subdirectory(cli)
add_subdirectory(irTool)
add_subdirectory(compilerTool)
