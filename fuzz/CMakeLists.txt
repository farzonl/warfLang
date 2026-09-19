# Copyright (c) 2023 F. Lotfi All rights reserved.
# Use of this source code is governed by a BSD-style
# license that can be found in the LICENSE file.

project(${CMAKE_PROJECT_NAME}_FUZZ)

include_directories(${CMAKE_SOURCE_DIR}/src/lib)

set(fuzzers "lexerFuzzer;parserFuzzer" CACHE STRING "libfuzzers")
foreach(fuzzer ${fuzzers})
  add_executable(${fuzzer} ${fuzzer}.cpp)
endforeach()

# Top-level CMakeLists.txt already applies -fsanitize=address,fuzzer-no-link
# project-wide (so WarfCore's instrumentation matches these harnesses); the
# harness executables additionally need the full fuzzer runtime + main.
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=fuzzer")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=fuzzer")

if(WIN32)
  # This project uses plain clang + lld-link (not clang-cl), so debug info
  # comes from -g/-gcodeview, not the cl.exe-style /DEBUG /Od flags.
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -Xclang -gcodeview")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Xclang -gcodeview")
elseif(UNIX)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
endif()

foreach(fuzzer ${fuzzers})
  target_link_libraries(${fuzzer} PRIVATE 
        WarfCore
  )
endforeach()
