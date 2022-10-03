
MSVC_VERSION="14.11.25547"
WIN_SDK_VERSION="10.0.22621.0"
MSVC_PATH=$PWD/win_sysroot/MSVC/$MSVC_VERSION/
WINSDK_PATH=$PWD/win_sysroot/WINSDK/$WIN_SDK_VERSION/

if [ ! -e $MSVC_PATH ]; then
    mkdir -p $MSVC_PATH/include
    mkdir -p $MSVC_PATH/lib

    mkdir win_sysroot_build
    pushd win_sysroot_build

    ZIP_NAME="msvc.zip"
    if [ ! -e $ZIP_NAME ]; then
        curl -L -O https://www.nuget.org/api/v2/package/VisualCppTools.Community.VS2017Layout/$MSVC_VERSION
        mv $MSVC_VERSION $ZIP_NAME
    fi
    mkdir msvc
    pushd msvc
    unzip -o ../msvc.zip
    rm -rf build package _rels VisualCppTools.Community.VS2017Layout.nuspec '[Content_Types].xml' lib/native/bin lib/native/redist/*.exe
    mv lib/native/lib/* $MSVC_PATH/lib
    mv lib/native/include/* $MSVC_PATH/include
    popd
    rm -rf msvc
    popd
fi

if [ ! -e $WINSDK_PATH ]; then
    mkdir -p $WINSDK_PATH/include
    mkdir -p $WINSDK_PATH/lib
    pushd win_sysroot_build
    if [[ ! -f "win10sdk.iso" ]]; then
        #windows 10.0.22621.0
        curl -L -o win10sdk.iso https://go.microsoft.com/fwlink/p/?linkid=2196240
    fi
    mkdir -p win10sdk_iso
    pushd win10sdk_iso
    7z x ../win10sdk.iso -aoa
    mkdir tmp
    msiextract -C tmp Installers/"Windows SDK Desktop Headers x64-x86_en-us.msi"
    msiextract -C tmp Installers/"Windows SDK Desktop Headers x86-x86_en-us.msi"
    msiextract -C tmp Installers/"Windows SDK Desktop Libs x64-x86_en-us.msi"
    msiextract -C tmp Installers/"Windows SDK Desktop Libs x86-x86_en-us.msi"
    msiextract -C tmp Installers/"Windows SDK for Windows Store Apps Headers-x86_en-us.msi"
    msiextract -C tmp Installers/"Windows SDK for Windows Store Apps Libs-x86_en-us.msi"
    msiextract -C tmp Installers/"Windows SDK for Windows Store Apps Tools-x86_en-us.msi"
    msiextract -C tmp Installers/"Windows SDK for Windows Store Apps Legacy Tools-x86_en-us.msi"
    msiextract -C tmp Installers/"Universal CRT Headers Libraries and Sources-x86_en-us.msi"

    mv "tmp/Program Files/Windows Kits/10/Lib/"$WIN_SDK_VERSION/* $WINSDK_PATH/lib
    mv "tmp/Program Files/Windows Kits/10/Include"/$WIN_SDK_VERSION/* $WINSDK_PATH/include
    popd
    #rm -rf win10sdk_iso
    popd
fi