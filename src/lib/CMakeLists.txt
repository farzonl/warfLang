# Copyright (c) 2023 F. Lotfi All rights reserved.
# Use of this source code is governed by a BSD-style
# license that can be found in the LICENSE file.

project(WarfCore)

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++latest /W4")
    # Default debug flags are OK 
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2")
else()
    set(CMAKE_CXX_STANDARD 17)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g")
endif()

# PathTracer Src dir
set(WARF_CORE_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})

set(SourceFiles Evaluator.cpp
)

set(HeaderFiles Evaluator.h
)

add_library (${PROJECT_NAME} STATIC
  WarfCoreStatic.cpp
  ${SourceFiles} ${HeaderFiles}
  $<TARGET_OBJECTS:WarfCore.Syntax>
  $<TARGET_OBJECTS:WarfCore.Binding>
  $<TARGET_OBJECTS:WarfCore.Symbol>
  $<TARGET_OBJECTS:WarfCore.Error>
  $<TARGET_OBJECTS:WarfCore.Version>
  $<TARGET_OBJECTS:WarfCore.DataStructures>
)

target_include_directories(${PROJECT_NAME}
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
  # Add include_directories here
  Syntax
  Binding
  Symbol
  Error
  Version
  DataStructures
)

add_subdirectory(Syntax)
add_subdirectory(Binding)
add_subdirectory(Symbol)
add_subdirectory(Error)
add_subdirectory(Version)
add_subdirectory(DataStructures)