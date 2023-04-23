# Copyright (c) 2023 F. Lotfi All rights reserved.
# Use of this source code is governed by a BSD-style
# license that can be found in the LICENSE file.

project(${CMAKE_PROJECT_NAME}_FUZZ)

include_directories(${CMAKE_SOURCE_DIR}/src/lib)

set(fuzzers "lexerFuzzer;parserFuzzer" CACHE STRING "libfuzzers")
foreach(fuzzer ${fuzzers})
  add_executable(${fuzzer} ${fuzzer}.cpp)
endforeach()


#clang++ -g -fsanitize=address,fuzzer

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fsanitize=address,fuzzer")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address,fuzzer")

if(Win32)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /DEBUG /Od")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /DEBUG /Od")
elseif(UNIX)
 set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g")
 set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g") 
endif()

foreach(fuzzer ${fuzzers})
  target_link_libraries(${fuzzer} PRIVATE 
        WarfCore
  )
endforeach()
