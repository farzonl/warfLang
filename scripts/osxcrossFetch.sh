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
    git checkout 7740274bd92dd6170a181653064791115a0e9d56
    popd
    popd
fi