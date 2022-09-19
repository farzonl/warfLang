if [ "$(expr substr $(uname -s) 1 5)" != "Linux" ]; then
    echo "This Script only works on Linux"
    exit 0
fi

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" >/dev/null 2>&1 && pwd )"

if [ ! -e $SCRIPT_DIR/../packages/darling_0.1.20220704.focal_amd64.deb ]; then
    
    apt-get update \
    && apt install -y \
    libavcodec58 libavformat58 libavutil56 \
    libcairo2 libdbus-1-3 libegl1 libgif7 libgl1 \
    libglu1-mesa libglu1 libpulse0 libswresample3 \
    libxcursor1 libxkbfile1 libxrandr2

    pushd $SCRIPT_DIR/../packages
    wget https://github.com/darlinghq/darling/releases/download/v0.1.20220704/darling_0.1.20220704.focal_amd64.deb
    dpkg -i darling_0.1.20220704.focal_amd64.deb
    popd
fi