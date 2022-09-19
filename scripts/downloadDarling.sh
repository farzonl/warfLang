#!/bin/bash

if [ "$(uname -s)" != "Linux" ]; then
    echo "This Script only works on Linux"
    exit 0
fi

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

if [ ! -e $SCRIPT_DIR/../packages/darling_0.1.20220704.focal_amd64.deb ]; then
    pushd $SCRIPT_DIR/../packages
    wget https://github.com/darlinghq/darling/releases/download/v0.1.20220704/darling_0.1.20220704.focal_amd64.deb
    popd
fi