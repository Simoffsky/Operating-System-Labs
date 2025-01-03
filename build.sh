#!/bin/bash

PROJECT_DIR=$(pwd)

echo "Updating git project"
git pull


BUILD_DIR="$PROJECT_DIR/build"
if [ ! -d "$BUILD_DIR" ]; then
    mkdir "$BUILD_DIR"
fi

cd "$BUILD_DIR"

echo "Building project..."
cmake ..
cmake --build .

echo "Build succeeded, builded project in $BUILD_DIR."
