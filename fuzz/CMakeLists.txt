project(${CMAKE_PROJECT_NAME}_FUZZ)

include_directories(${CMAKE_SOURCE_DIR}/src/lib)

add_executable(${PROJECT_NAME} fuzzTest.cpp)

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


target_link_libraries(${PROJECT_NAME} PRIVATE 
        WarfCore
)