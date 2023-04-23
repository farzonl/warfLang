# Copyright (c) 2023 F. Lotfi All rights reserved.
# Use of this source code is governed by a BSD-style
# license that can be found in the LICENSE file.

project(Warf)

set(SOURCE_FILES
                 # add files here
                 main.cpp)

if(WIN32)
list(APPEND SOURCE_FILES
    ${CMAKE_CURRENT_BINARY_DIR}/../lib/Version/VersionResource.rc
    ${CMAKE_CURRENT_BINARY_DIR}/../lib/Version/VersionInfo.h
)
endif()

include_directories(${CMAKE_SOURCE_DIR}/src/lib)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

set(TargetLibraries "")

if(DISABLE_LIBEDIT)
add_definitions(-DDISABLE_LIBEDIT=1)
endif()


if(UNIX AND NOT isEmscripten AND NOT DISABLE_LIBEDIT)
    list(APPEND TargetLibraries
        "-ledit"
    )
    # Note we really only need this for the arm64 crossbuild
    if(LINUX)
        list(APPEND TargetLibraries
            "-lbsd"
        )
        list(APPEND TargetLibraries
            "-ltinfo"
        )
    endif()
endif()

if(isEmscripten AND CMAKE_BUILD_TYPE MATCHES Debug)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -gseparate-dwarf=${SOURCE_FILES}.debug.wasm")
endif()

target_link_libraries (${PROJECT_NAME}
  WarfCore
  ${TargetLibraries}
)

target_include_directories(${PROJECT_NAME}
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
  # Add include_directories here
)