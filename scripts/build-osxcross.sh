#!/bin/bash

if [ "$(uname -s)" != "Linux" ]; then
    echo "This Script only works on Linux"
    exit 0
fi

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
if [ ! -e $SCRIPT_DIR/../packages/osxcross/target/bin/oa64-clang ]; then
    pushd $SCRIPT_DIR/../packages/osxcross
    UNATTENDED=1 ./build.sh
    popd
fi
