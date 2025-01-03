@echo off

set PROJECT_DIR=%cd%
set BUILD_DIR=%PROJECT_DIR%\build


echo Updating git project...

git pull || exit /b

if not exist "%BUILD_DIR%" (
    mkdir "%BUILD_DIR%"
)

cd "%BUILD_DIR%"

echo Building project...
cmake -G "MinGW Makefiles" .. || exit /b
cmake --build . || exit /b

echo Build succeeded, builded project in %BUILD_DIR%.

