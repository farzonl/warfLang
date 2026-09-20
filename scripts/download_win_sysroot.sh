#!/bin/bash
set -euo pipefail

# Downloads a portable MSVC CRT + Windows SDK sysroot for cross-compiling to
# Windows with clang/lld-link, using xwin (https://github.com/Jake-Shadle/xwin).
#
# xwin pulls the CRT/SDK straight from Microsoft's official installer manifest,
# so it's always a current, fully-supported toolset (unlike the old approach
# here, which vendored an unlisted, October-2017 MSVC 14.11 snapshot that
# predates vcruntime's dynamic TLS-init support and can't build code using
# thread_local with a non-trivial constructor).
XWIN_VERSION_OVERRIDE="${XWIN_VERSION:-}"
SYSROOT_DIR="$PWD/win_sysroot"
XWIN_CACHE_DIR="$PWD/.xwin-cache"
XWIN_BIN_DIR="$PWD/.xwin-bin"
XWIN_BIN="$XWIN_BIN_DIR/xwin"

if [ -e "$SYSROOT_DIR/crt" ] && [ -e "$SYSROOT_DIR/sdk" ]; then
    echo "win_sysroot already present, skipping download."
    exit 0
fi

if command -v xwin >/dev/null 2>&1; then
    # Already available (e.g. `brew install xwin` on macOS).
    XWIN_BIN="$(command -v xwin)"
elif [ "$(uname -s)" = "Darwin" ]; then
    # No prebuilt tarball is published for macOS; Homebrew has an official
    # formula for it instead.
    brew install xwin
    XWIN_BIN="$(command -v xwin)"
else
    if [ -n "$XWIN_VERSION_OVERRIDE" ]; then
        XWIN_TAG="$XWIN_VERSION_OVERRIDE"
    else
        XWIN_TAG=$(curl -sL https://api.github.com/repos/Jake-Shadle/xwin/releases/latest | grep -o '"tag_name": *"[^"]*"' | head -n1 | sed -E 's/.*"([^"]+)"$/\1/')
    fi

    case "$(uname -s)-$(uname -m)" in
        Linux-x86_64)   XWIN_ASSET="xwin-$XWIN_TAG-x86_64-unknown-linux-musl.tar.gz" ;;
        Linux-aarch64)  XWIN_ASSET="xwin-$XWIN_TAG-aarch64-unknown-linux-musl.tar.gz" ;;
        *) echo "Unsupported host for xwin: $(uname -s)-$(uname -m)" >&2; exit 1 ;;
    esac

    if [ ! -x "$XWIN_BIN" ]; then
        mkdir -p "$XWIN_BIN_DIR"
        curl -L -o /tmp/xwin.tar.gz "https://github.com/Jake-Shadle/xwin/releases/download/$XWIN_TAG/$XWIN_ASSET"
        tar -xzf /tmp/xwin.tar.gz -C "$XWIN_BIN_DIR" --strip-components=1
        rm -f /tmp/xwin.tar.gz
    fi
fi

"$XWIN_BIN" --accept-license --cache-dir "$XWIN_CACHE_DIR" splat \
    --output "$SYSROOT_DIR" \
    --preserve-ms-arch-notation \
    --include-debug-libs
