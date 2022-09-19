if [ "$(expr substr $(uname -s) 1 5)" != "Linux" ]; then
    echo "This Script only works on Linux"
    exit 0
fi

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"
if [ ! -e $SCRIPT_DIR/../packages/osxcross ]; then
    pushd $SCRIPT_DIR/../packages/
    git clone https://github.com/tpoechtrager/osxcross
    pushd tools
    ./get_dependencies.sh
    popd
    pushd tarballs
    wget https://github.com/phracker/MacOSX-SDKs/releases/download/11.3/MacOSX11.3.sdk.tar.xz
    popd
    UNATTENDED=1 ./build.sh 
    popd
    export PATH=$PATH:/opt/osxcross/bin
    export LD_LIBRARY_PATH=/opt/osxcross/lib:$LD_LIBRARY_PATH
fi