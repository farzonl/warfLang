# Copyright (c) 2023 F. Lotfi All rights reserved.
# Use of this source code is governed by a BSD-style
# license that can be found in the LICENSE file.

project(WarfWasm)

set(SOURCE_FILES
                 # add files here
                 wasmEntry.cpp)

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/test.html
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/index.html
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

set(exports 
   _InitWarf 
   _RunWarf
   _ShowSyntaxTree
   _malloc
   _free
 )

set(libEm_exports 
   Pointer_stringify
   stringToUTF8
 )

# process exported functions
set(exports_string "")
list(JOIN exports "," exports_string)
set(libEm_exports_string "")
list(JOIN libEm_exports "," libEm_exports_string)


# specify the project
add_executable(${PROJECT_NAME} ${SOURCE_FILES})

target_link_options(${PROJECT_NAME} PRIVATE
        -s EXPORTED_FUNCTIONS=[${exports_string}])

#target_link_options(${PROJECT_NAME} PRIVATE
#        -s DEFAULT_LIBRARY_FUNCS_TO_INCLUDE=[${libEm_exports_string}])

target_link_libraries (${PROJECT_NAME}
  WarfCore
  ${TargetLibraries}
)

target_include_directories(${PROJECT_NAME}
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
  # Add include_directories here
)