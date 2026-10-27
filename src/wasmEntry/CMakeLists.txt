# Copyright (c) 2023 F. Lotfi All rights reserved.
# Use of this source code is governed by a BSD-style
# license that can be found in the LICENSE file.

project(WarfWasm)

set(SOURCE_FILES
                 # add files here
                 wasmEntry.cpp)

file(GLOB WASM_WEB_ASSETS
     LIST_DIRECTORIES false
     "${CMAKE_CURRENT_SOURCE_DIR}/*.html"
     "${CMAKE_CURRENT_SOURCE_DIR}/*.css"
     "${CMAKE_CURRENT_SOURCE_DIR}/*.js")
if(WASM_WEB_ASSETS)
  file(COPY ${WASM_WEB_ASSETS} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
endif()

set(exports 
   _InitWarf 
   _RunWarf
   _run_warf
   _emit_warf_ir
   _ShowSyntaxTree
   _ShowIR
   _AnalyzeWarf
   _ExecuteWarf
   _malloc
   _free
 )

set(runtime_exports 
   HEAPU8
   UTF8ToString
   stringToUTF8
   lengthBytesUTF8
   ccall
   cwrap
 )

# process exported functions
set(exports_string "")
list(JOIN exports "," exports_string)
set(runtime_exports_string "")
list(JOIN runtime_exports "," runtime_exports_string)


# specify the project
add_executable(${PROJECT_NAME} ${SOURCE_FILES})

target_link_options(${PROJECT_NAME} PRIVATE
        "-sEXPORTED_FUNCTIONS=[${exports_string}]"
        "-sEXPORTED_RUNTIME_METHODS=[${runtime_exports_string}]"
        "-sALLOW_MEMORY_GROWTH=1")

target_link_libraries (${PROJECT_NAME}
  WarfCore
  ${TargetLibraries}
)

target_include_directories(${PROJECT_NAME}
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
  # Add include_directories here
)
