# Copyright (c) 2023 F. Lotfi All rights reserved.
# Use of this source code is governed by a BSD-style
# license that can be found in the LICENSE file.

# this file is intended to be included by other project CMakeLists.txt files
# in order to make the following Warf version variables available
#   WARF_MAJOR : Integer major number (e.g. 0, 1, 2)
#   WARF_MINOR : Integer minor number (e.g. 0, 1, 2)
#   WARF_PATCH : Integer patch number (e.g. 0, 1, 2)
#   WARF_VERSION : Full WARF verion string as MAJOR.MINOR.PATCH+BUILD (e.g. 1.0.5+255)

file (READ ${CMAKE_CURRENT_LIST_DIR}/version.txt VERSION NEWLINE_CONSUME)
string (REPLACE "." ";" VERSION "${VERSION}")
string (REPLACE "\n" ";" VERSION "${VERSION}")
string (REPLACE "\r" ";" VERSION "${VERSION}")
list (GET VERSION 0 WARF_MAJOR)
list (GET VERSION 1 WARF_MINOR)
list (GET VERSION 2 WARF_PATCH)

file (READ ${CMAKE_CURRENT_LIST_DIR}/appName.txt WARF_NAME)


set (WARF_VERSION "${WARF_MAJOR}.${WARF_MINOR}.${WARF_PATCH}")