cmake_minimum_required(VERSION 3.15)

project(HelloWorld LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(hello_world main.cpp)

if(WIN32)
    message(STATUS "Configuring for Windows")
    set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)
elseif(APPLE)
    message(STATUS "Configuring for macOS")
    set(CMAKE_MACOSX_RPATH ON)
else()
    message(STATUS "Configuring for Linux")
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif()


target_compile_options(hello_world PRIVATE -Wall -Wextra -pedantic)

install(TARGETS hello_world
        RUNTIME DESTINATION bin)
