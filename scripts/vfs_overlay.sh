#!/bin/bash

# Create a VFS overlay of the Win SDK for a case insensitive file system
echo "version: 0" > winsdk_vfs_overlay.yaml
echo "case-sensitive: false" >> winsdk_vfs_overlay.yaml
echo "roots:" >> winsdk_vfs_overlay.yaml

MSVC_VERSION="14.11.25547"
WIN_SDK_VERSION="10.0.22621.0"


for dir in $(find $PWD/win_sysroot/WINSDK/$WIN_SDK_VERSION/include/ -type d); do
    files=$(find $dir -maxdepth 1 -name "*.h")
    if [[ "$files" != "" ]]; then
        echo "  - name: \"$dir\"" >> winsdk_vfs_overlay.yaml
        echo "    type: directory" >> winsdk_vfs_overlay.yaml
        echo "    contents:" >> winsdk_vfs_overlay.yaml
        for f in $files; do
            echo "      - name: \"$(basename $f)\"" >> winsdk_vfs_overlay.yaml
            echo "        type: file" >> winsdk_vfs_overlay.yaml
            echo "        external-contents: \"$f\"" >> winsdk_vfs_overlay.yaml
        done
    fi
done

for dir in $(find $PWD/win_sysroot/MSVC/$MSVC_VERSION/include/ -type d); do
    files=$(find $dir -maxdepth 1 -name "*.h")
    if [[ "$files" != "" ]]; then
        echo "  - name: \"$dir\"" >> winsdk_vfs_overlay.yaml
        echo "    type: directory" >> winsdk_vfs_overlay.yaml
        echo "    contents:" >> winsdk_vfs_overlay.yaml
        for f in $files; do
            echo "      - name: \"$(basename $f)\"" >> winsdk_vfs_overlay.yaml
            echo "        type: file" >> winsdk_vfs_overlay.yaml
            echo "        external-contents: \"$f\"" >> winsdk_vfs_overlay.yaml
        done
    fi
done