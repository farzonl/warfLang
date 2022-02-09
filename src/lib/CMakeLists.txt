project(WarfCore)
cmake_minimum_required(VERSION 3.10)

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

add_library (${PROJECT_NAME} STATIC
  WarfCoreStatic.cpp
  $<TARGET_OBJECTS:WarfCore.Syntax>
  $<TARGET_OBJECTS:WarfCore.Binding>
)

add_subdirectory(Syntax)
add_subdirectory(Binding)