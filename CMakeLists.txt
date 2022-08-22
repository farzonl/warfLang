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

cmake_minimum_required(VERSION 3.23)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory (src)
add_subdirectory (test)

include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-fsanitize=address,fuzzer" HAVE_SANITIZE_FUZZER)
if(HAVE_SANITIZE_FUZZER)
  add_subdirectory (fuzz)
endif()
