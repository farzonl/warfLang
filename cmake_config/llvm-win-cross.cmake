
if(APPLE)
    set(CMAKE_C_COMPILER "/usr/local/opt/llvm/bin/clang")
    set(CMAKE_CXX_COMPILER "/usr/local/opt/llvm/bin/clang++")
    set(CMAKE_RC_COMPILER "/usr/local/opt/llvm/bin/llvm-rc")
elseif(UNIX AND NOT APPLE)
    set(CMAKE_C_COMPILER "clang")
    set(CMAKE_CXX_COMPILER "clang++")
    set(CMAKE_LINKER "lld")
    set(CMAKE_RC_COMPILER "llvm-rc")
endif()

set(TARGET_TRIPLE "x86_64-pc-win32")

set(CMAKE_EXE_LINKER_FLAGS_INIT "-fuse-ld=lld")
set(CMAKE_MODULE_LINKER_FLAGS_INIT "-fuse-ld=lld")
set(CMAKE_SHARED_LINKER_FLAGS_INIT "-fuse-ld=lld")

set(CMAKE_SYSTEM_NAME "Windows")
Set(CMAKE_CROSS_COMPILING TRUE)
set(CMAKE_C_COMPILER_TARGET ${TARGET_TRIPLE})
set(CMAKE_CXX_COMPILER_TARGET ${TARGET_TRIPLE})
set(CMAKE_ASM_COMPILER_TARGET ${TARGET_TRIPLE})
set(MSVC_VERSION "14.11.25547")
set(WIN_SDK_VERSION "10.0.22621.0")
set(WIN_SDK_PATH  "win_sysroot/WINSDK/${WIN_SDK_VERSION}")
set(WIN_MSVC_PATH "win_sysroot/MSVC/${MSVC_VERSION}")
set(WIN_SDK_INCLUDE_PATH "${WIN_SDK_PATH}/include")
set(WIN_SDK_LIB_PATH "${WIN_SDK_PATH}/lib")
set(WiN_VFS_OVERLAY ${CMAKE_CURRENT_LIST_DIR}/../packages/winsdk_vfs_overlay.yaml)

if(APPLE)
    include_directories(/usr/local/opt/llvm/include)
elseif(UNIX AND NOT APPLE)
    include_directories(/usr/include/llvm)
endif()

include_directories(
    ${CMAKE_CURRENT_LIST_DIR}/../packages/${WIN_MSVC_PATH}/include
    ${CMAKE_CURRENT_LIST_DIR}/../packages/${WIN_SDK_INCLUDE_PATH}/um
    ${CMAKE_CURRENT_LIST_DIR}/../packages/${WIN_SDK_INCLUDE_PATH}/ucrt
    ${CMAKE_CURRENT_LIST_DIR}/../packages/${WIN_SDK_INCLUDE_PATH}/shared
    ${CMAKE_CURRENT_LIST_DIR}/../packages/${WIN_SDK_INCLUDE_PATH}/winrt)

link_directories(
                 ${CMAKE_CURRENT_LIST_DIR}/../packages/${WIN_MSVC_PATH}/lib/x64
                 ${CMAKE_CURRENT_LIST_DIR}/../packages/${WIN_SDK_LIB_PATH}/um/x64
                 ${CMAKE_CURRENT_LIST_DIR}/../packages/${WIN_SDK_LIB_PATH}/ucrt/x64)

add_compile_options(
    # XCLANG_FLAGS
    "SHELL:-Xclang -ivfsoverlay -Xclang ${WiN_VFS_OVERLAY}"
    "SHELL:-Xclang --dependent-lib=oldnames"
    "SHELL:-Xclang --dependent-lib=ucrt"
    "SHELL:-Xclang --dependent-lib=vcruntime"
    "SHELL:-Xclang --dependent-lib=msvcrt$<$<CONFIG:Debug>:d>"
    "SHELL:-Xclang -disable-llvm-verifier"
    # WARNINGS
    -Wno-msvc-not-found
    # GNU undefs
    "-U__GNUC__"
    "-U__gnu_linux__" 
    "-U__GNUC_MINOR__" 
    "-U__GNUC_PATCHLEVEL__" 
    "-U__GNUC_STDC_INLINE__"
    #Definitions
    "-DWIN32" 
    "-D_WIN#@"
    "-D_MT" 
    "-D_DLL" 
    "-D_CRT_SECURE_NO_WARNINGS" 
    "-D_CRT_NONSTDC_NO_DEPRECATE"
)

set(F_FLAGS "-fms-compatibility-version=19.11 -fms-extensions -fdelayed-template-parsing -fexceptions -mthread-model posix -fno-threadsafe-statics")
set(COMPILE_FLAGS "--target=${TARGET_TRIPLE} -nostdlib -lmsvcrt ${F_FLAGS}" )
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}  ${COMPILE_FLAG}" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMPILE_FLAG}" CACHE STRING "" FORCE)

set(CMAKE_RC_FLAGS "${CMAKE_RC_FLAGS} -Xclang -ivfsoverlay -Xclang ${WiN_VFS_OVERLAY}")