# Copyright (c) 2023 F. Lotfi All rights reserved.
# Use of this source code is governed by a BSD-style
# license that can be found in the LICENSE file.

cmake_minimum_required(VERSION 3.22)
project(WarfLang)

function(download_file url filename hash_type hash)

if(NOT EXISTS ${filename})
  file(DOWNLOAD ${url} ${filename}
       TIMEOUT 60  # seconds
       EXPECTED_HASH ${hash_type}=${hash}
       TLS_VERIFY ON
      )
endif()

endfunction(download_file)

if(UNIX AND NOT APPLE)
    set(LINUX TRUE)
endif()

if(WIN32  AND CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  # See https://github.com/Kitware/CMake/blob/master/Modules/Platform/Windows-Clang.cmake#L86
  #This will set:
  # For Release: -fms-runtime-lib=static -Xclang -flto-visibility-public-std -Xclang --dependent-lib=libcmt
  # For Debug: -fms-runtime-lib=static_dbg -Xclang -flto-visibility-public-std -Xclang --dependent-lib=libcmtd
  set(CMAKE_MSVC_RUNTIME_LIBRARY_DEFAULT "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
    # Default debug flags are OK 
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
else()
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g")
endif()

if(CMAKE_CROSS_COMPILING)
  if(CMAKE_SYSTEM_NAME MATCHES Windows)
    set(WIN32 TRUE)
    set(LINUX FALSE)
    set(UNIX FALSE)
  endif()
  if(CMAKE_SYSTEM_NAME MATCHES Linux)
    set(WIN32 FALSE)
    set(LINUX TRUE)
    set(UNIX TRUE)

  endif()
  if(CMAKE_SYSTEM_NAME MATCHES MacOS)
    set(WIN32 FALSE)
    set(LINUX FALSE)
    set(APPLE TRUE)
    set(UNIX TRUE)
  endif()
  if(CMAKE_SYSTEM_NAME MATCHES ".*(BSD)")
    set(WIN32 FALSE)
    set(LINUX FALSE)
    set(APPLE FALSE)
    set(UNIX TRUE)
  endif()
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if ("${_cmake_compiler_output}" MATCHES "[Ee]mscripten")
  set(isEmscripten TRUE)
  set(CMAKE_EXECUTABLE_SUFFIX ".js")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DEmscripten")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DEmscripten")
endif()

if(BUILD_ASAN)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address")
endif()

if (BUILD_FUZZER AND CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND NOT isEmscripten )
  add_subdirectory (fuzz)
endif()

add_subdirectory (src)
if(BuildTest)
  add_subdirectory (test)
endif()
