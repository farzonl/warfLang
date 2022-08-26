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


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if ("${_cmake_compiler_output}" MATCHES "[Ee]mscripten")
  set(isEmscripten TRUE)
  set(CMAKE_EXECUTABLE_SUFFIX ".html")
  add_definitions(-DisEmscripten=1)
  #set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -sEXPORTED_FUNCTIONS=run -sEXPORTED_RUNTIME_METHODS=ccall,cwrap")
  #set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -sEXPORTED_FUNCTIONS=run -sEXPORTED_RUNTIME_METHODS=ccall,cwrap")
endif()

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang" AND NOT isEmscripten )
  add_subdirectory (fuzz)
endif()

add_subdirectory (src)
add_subdirectory (test)
