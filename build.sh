#!/bin/bash

set -e

PROJECT_DIR=$(pwd)

echo "Обновление исходников из Git..."
git pull


BUILD_DIR="$PROJECT_DIR/build"
if [ ! -d "$BUILD_DIR" ]; then
    mkdir "$BUILD_DIR"
fi

cd "$BUILD_DIR"

echo "Сборка проекта..."
cmake ..
cmake --build .

echo "Сборка завершена. Исполняемый файл находится в папке $BUILD_DIR."
