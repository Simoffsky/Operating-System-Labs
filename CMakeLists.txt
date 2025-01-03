cmake_minimum_required(VERSION 3.15)

project(LibraryProject LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

add_subdirectory(library)

set(EXAMPLE_NAME example)
add_executable(${EXAMPLE_NAME} example.c)

target_link_libraries(${EXAMPLE_NAME} PRIVATE library)
