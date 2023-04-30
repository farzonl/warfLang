# Copyright (c) 2023 F. Lotfi All rights reserved.
# Use of this source code is governed by a BSD-style
# license that can be found in the LICENSE file.

add_subdirectory(lib)
if(isEmscripten)
    add_subdirectory(wasmEntry)
endif()
add_subdirectory(cli)
