#!/bin/bash

set -e
BUILD_TYPE="Debug" # default build is debug

if [ $# -ge 1 ]; then
    arg=$(echo "$1" | tr '[:upper:]' '[:lower:]')

    if [[ "$arg" == "debug" ]]; then
        BUILD_TYPE="Debug"
        echo "[ -- ] Building in debug mode"

        echo "Using "
    elif [[ "$arg" == "release" ]]; then
        BUILD_TYPE="Release"
        echo "[ -- ] Building in release mode"
    else
        echo "Invalid argument: $1"
        echo "Usage: $0 [release|debug]"
        exit 1
    fi
fi


echo "[ -- ] Creating necessary directories..."
mkdir -p "build"
mkdir -p "cache/textures"

cd "build"

echo "[ -- ] Running CMake configuration..."

cmake .. -DCMAKE_BUILD_TYPE=$BUILD_TYPE

echo "[ -- ] Building with maximum available cores..."
cmake --build . -- -j$(nproc 2>/dev/null || echo 10)
