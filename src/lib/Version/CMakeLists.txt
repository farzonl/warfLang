# Copyright (c) 2023 F. Lotfi All rights reserved.
# Use of this source code is governed by a BSD-style
# license that can be found in the LICENSE file.



include ("CMakeListsVersion.cmake")

set(GIT_EXECUTABLE git)
set(WARF_DESCRIPTION "Warf, An Interpreter for the Warf Language.")

# the commit's SHA1, and whether the building workspace was dirty or not
execute_process(COMMAND
  "${GIT_EXECUTABLE}" rev-parse --short HEAD
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
  OUTPUT_VARIABLE GIT_SHA1
  ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)

# the date of the commit
execute_process(COMMAND
  "${GIT_EXECUTABLE}" log -1 --format=%ad --date=local
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
  OUTPUT_VARIABLE GIT_DATE
  ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)

# the subject of the commit
execute_process(COMMAND
  "${GIT_EXECUTABLE}" log -1 --format=%s
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
  OUTPUT_VARIABLE GIT_COMMIT_SUBJECT
  ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)

# the branch of the commit
execute_process(COMMAND
  "${GIT_EXECUTABLE}" branch --show-current
  WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
  OUTPUT_VARIABLE GIT_COMMIT_BRANCH
  ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)


# generate version.cpp
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/version.cpp.in" 
               "${CMAKE_CURRENT_BINARY_DIR}/version.cpp" @ONLY)

set (SourceFiles
    ${CMAKE_CURRENT_BINARY_DIR}/version.cpp
)

if(WIN32)
  include(generate_product_version.cmake)
  file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/icon.ico
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
  generate_product_version(
   VersionFilesOutputVariable
   NAME "${WARF_NAME}"
   ICON ${CMAKE_CURRENT_BINARY_DIR}/icon.ico
   VERSION_MAJOR ${WARF_MAJOR}
   VERSION_MINOR ${WARF_MINOR}
   VERSION_PATCH ${WARF_PATCH}
   #VERSION_REVISION 0 #could hook this up with the cli
   COMPANY_NAME ${WARF_NAME}
   FILE_DESCRIPTION ${WARF_DESCRIPTION}
   )
endif()


set(HeaderFiles
    version.h
)

add_library (WarfCore.Version OBJECT
            ${SourceFiles} ${HeaderFiles}
)
set_target_properties(WarfCore.Version PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories (
    WarfCore.Version PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)
