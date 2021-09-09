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

cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory (src)
add_subdirectory (fuzz)
add_subdirectory (test)
