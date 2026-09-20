
if(APPLE)
    set(CMAKE_C_COMPILER "/usr/local/opt/llvm/bin/clang")
    set(CMAKE_CXX_COMPILER "/usr/local/opt/llvm/bin/clang++")
    set(CMAKE_RC_COMPILER "/usr/local/opt/llvm/bin/llvm-rc")
    set(LLVM_LINKER_NAME "lld")
elseif(UNIX AND NOT APPLE)
    # Use the versioned binaries directly: Ubuntu runner images can already
    # have an unversioned "clang" alternative registered at a higher priority
    # than the one the CI install step's LLVM install would provide, so the
    # bare name is not reliable here. The MSVC STL headers in the xwin
    # sysroot require Clang 19+ (STL1000 static_assert); LLVM_VERSION is
    # passed in by the workflow (detected from the latest LLVM install),
    # defaulting to 19 for local/manual configures.
    if(NOT DEFINED LLVM_VERSION)
        set(LLVM_VERSION 19)
    endif()
    set(CMAKE_C_COMPILER "clang-${LLVM_VERSION}")
    set(CMAKE_CXX_COMPILER "clang++-${LLVM_VERSION}")
    set(CMAKE_RC_COMPILER "llvm-rc-${LLVM_VERSION}")
    set(LLVM_LINKER_NAME "lld-${LLVM_VERSION}")
endif()

set(TARGET_TRIPLE "x86_64-pc-win32")

set(CMAKE_EXE_LINKER_FLAGS_INIT "-fuse-ld=${LLVM_LINKER_NAME}")
set(CMAKE_MODULE_LINKER_FLAGS_INIT "-fuse-ld=${LLVM_LINKER_NAME}")
set(CMAKE_SHARED_LINKER_FLAGS_INIT "-fuse-ld=${LLVM_LINKER_NAME}")

set(CMAKE_SYSTEM_NAME "Windows")
Set(CMAKE_CROSS_COMPILING TRUE)
set(CMAKE_C_COMPILER_TARGET ${TARGET_TRIPLE})
set(CMAKE_CXX_COMPILER_TARGET ${TARGET_TRIPLE})
set(CMAKE_ASM_COMPILER_TARGET ${TARGET_TRIPLE})

# Sysroot produced by scripts/download_win_sysroot.sh via xwin
# (https://github.com/Jake-Shadle/xwin), which fetches a current MSVC CRT +
# Windows SDK directly from Microsoft's installer manifest and handles the
# Windows SDK's header-casing issues natively (via symlinks), so no VFS
# overlay/renaming pass is needed here anymore.
set(WIN_SYSROOT ${CMAKE_CURRENT_LIST_DIR}/../packages/win_sysroot)

if(APPLE)
    include_directories(/usr/local/opt/llvm/include)
elseif(UNIX AND NOT APPLE)
    include_directories(/usr/include/llvm)
endif()

include_directories(
    ${WIN_SYSROOT}/crt/include
    ${WIN_SYSROOT}/sdk/include/ucrt
    ${WIN_SYSROOT}/sdk/include/um
    ${WIN_SYSROOT}/sdk/include/shared
    ${WIN_SYSROOT}/sdk/include/winrt)

link_directories(
    ${WIN_SYSROOT}/crt/lib/x64
    ${WIN_SYSROOT}/sdk/lib/um/x64
    ${WIN_SYSROOT}/sdk/lib/ucrt/x64)

set(GNU_UNDEF_FLAGS "-U__GNUC__ -U__gnu_linux__ -U__GNUC_MINOR__ -U__GNUC_PATCHLEVEL__ -U__GNUC_STDC_INLINE__")
set(F_FLAGS "-fms-extensions -fdelayed-template-parsing -fexceptions -fno-threadsafe-statics")
set(DEF_FLAGS "-DWIN32 -D_CRT_SECURE_NO_WARNINGS -D_CRT_NONSTDC_NO_DEPRECATE")
set(WARN_FLAGS "-Wno-msvc-not-found")
set(COMPILE_FLAGS "--target=${TARGET_TRIPLE} ${F_FLAGS} ${DEF_FLAGS} ${GNU_UNDEF_FLAGS} ${WARN_FLAGS}")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${COMPILE_FLAGS}" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${COMPILE_FLAGS}" CACHE STRING "" FORCE)

# CMAKE_MSVC_RUNTIME_LIBRARY_DEFAULT (top-level CMakeLists.txt) selects the
# static libcmt/libcmtd runtime and embeds it automatically via --dependent-lib.
# Add the matching static ucrt/vcruntime support libs a native MSVC static
# (/MT or /MTd) link pulls in automatically, so thread_local globals with
# non-trivial constructors (__dyn_tls_on_demand_init / __tls_guard) resolve.
set(DEPENDENT_LIB_FLAGS_COMMON "-Xclang --dependent-lib=oldnames")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -Xclang --dependent-lib=libucrtd -Xclang --dependent-lib=libvcruntimed ${DEPENDENT_LIB_FLAGS_COMMON}" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Xclang --dependent-lib=libucrtd -Xclang --dependent-lib=libvcruntimed ${DEPENDENT_LIB_FLAGS_COMMON}" CACHE STRING "" FORCE)
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Xclang --dependent-lib=libucrt -Xclang --dependent-lib=libvcruntime ${DEPENDENT_LIB_FLAGS_COMMON}" CACHE STRING "" FORCE)
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -Xclang --dependent-lib=libucrt -Xclang --dependent-lib=libvcruntime ${DEPENDENT_LIB_FLAGS_COMMON}" CACHE STRING "" FORCE)
