# Copyright (c) 2023 F. Lotfi All rights reserved.
# Use of this source code is governed by a BSD-style
# license that can be found in the LICENSE file.

project(${CMAKE_PROJECT_NAME}_TEST)

download_file(https://raw.githubusercontent.com/onqtam/doctest/2.4.6/doctest/doctest.h 
              ${CMAKE_SOURCE_DIR}/packages/doctest/doctest.h
              MD5
              1494db8a28035bcf6caeea817c4b2b65)

include_directories(${CMAKE_SOURCE_DIR}/packages/
                    ${CMAKE_SOURCE_DIR}/src/lib
)

add_executable(${PROJECT_NAME}
               #$<TARGET_OBJECTS:WarfLang.ParserTest>
               #$<TARGET_OBJECTS:WarfLang.LexerTest>
               #$<TARGET_OBJECTS:WarfLang.ASTTest>
               test.cpp
)

target_link_libraries (${PROJECT_NAME}
  WarfCore
)

#add_subdirectory(parser)
#add_subdirectory(lexer)
#add_subdirectory(ast)

if(WARF_ENABLE_FILECHECK_TESTS)
  find_package(Python3 COMPONENTS Interpreter QUIET)
  find_program(WARF_FILECHECK_EXECUTABLE NAMES FileCheck filecheck)
  find_program(WARF_LIT_EXECUTABLE NAMES lit llvm-lit)

  if(NOT Python3_Interpreter_FOUND)
    message(FATAL_ERROR "WARF_ENABLE_FILECHECK_TESTS is ON, but Python 3 was not found")
  endif()
  if(NOT WARF_FILECHECK_EXECUTABLE)
    message(FATAL_ERROR "WARF_ENABLE_FILECHECK_TESTS is ON, but FileCheck was not found. Install it with: python3 -m pip install filecheck")
  endif()
  if(NOT WARF_LIT_EXECUTABLE)
    message(FATAL_ERROR "WARF_ENABLE_FILECHECK_TESTS is ON, but lit was not found. Install it with: python3 -m pip install lit")
  endif()

  file(GENERATE
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/filecheck/lit.site.cfg.py
    CONTENT "config.test_exec_root = r\"${CMAKE_CURRENT_BINARY_DIR}/filecheck\"\nconfig.substitutions.append((\"%warf\", r\"$<TARGET_FILE:Warf>\"))\nconfig.substitutions.append((\"%FileCheck\", r\"${WARF_FILECHECK_EXECUTABLE}\"))\n"
  )

  set(WARF_LIT_ENV
      ${CMAKE_COMMAND} -E env
      WARF_LIT_SITE_CONFIG=${CMAKE_CURRENT_BINARY_DIR}/filecheck/lit.site.cfg.py
  )

  function(add_warf_lit_suite suite_name suite_dir)
    add_custom_target(check-${suite_name}
      COMMAND ${WARF_LIT_ENV} ${WARF_LIT_EXECUTABLE} -sv ${CMAKE_CURRENT_SOURCE_DIR}/filecheck/${suite_dir}
      DEPENDS Warf
      USES_TERMINAL
    )
  endfunction()

  add_warf_lit_suite(warf-ast warf-ast)
  add_warf_lit_suite(warf-driver warf-driver)
  add_warf_lit_suite(warf-sema warf-sema)
  add_warf_lit_suite(warf-exec warf-exec)

  add_custom_target(check-warf-cli
    COMMAND ${WARF_LIT_ENV} ${WARF_LIT_EXECUTABLE} -sv ${CMAKE_CURRENT_SOURCE_DIR}/filecheck
    DEPENDS Warf
    USES_TERMINAL
  )
endif()
