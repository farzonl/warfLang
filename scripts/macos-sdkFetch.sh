#!/bin/bash

if [ "$(expr substr $(uname -s) 1 5)" != "Linux" ]; then
    echo "This Script only works on Linux"
    exit 0
fi

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
if [ ! -e $SCRIPT_DIR/../packages/osxcross/tarballs/MacOSX11.3.sdk.tar.xz ]; then
    pushd $SCRIPT_DIR/../packages/osxcross/tarballs
    wget https://github.com/phracker/MacOSX-SDKs/releases/download/11.3/MacOSX11.3.sdk.tar.xz
    popd
fi