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
endif()

target_link_libraries (${PROJECT_NAME}
  WarfCore
  ${TargetLibraries}
)

target_include_directories(${PROJECT_NAME}
  PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
  # Add include_directories here
)