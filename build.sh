#!/bin/bash

set -e

echo "Creating necessary directories..."
mkdir -p "build"
mkdir -p "cache/textures"

cd "build"

echo "Running CMake configuration..."
cmake ..

echo "Building with maximum available cores..."
cmake --build . -- -j$(nproc 2>/dev/null || echo 10)
