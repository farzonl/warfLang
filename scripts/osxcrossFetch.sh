#!/bin/bash

if [ "$(uname -s)" != "Linux" ]; then
    echo "This Script only works on Linux"
    exit 0
fi

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
if [ ! -e $SCRIPT_DIR/../packages/osxcross ]; then
    pushd $SCRIPT_DIR/../packages/
    git clone https://github.com/tpoechtrager/osxcross
    pushd osxcross
    git checkout 26063b07e2aa6a32ba4be31933fd7bda45886d22
    popd
    popd
fi