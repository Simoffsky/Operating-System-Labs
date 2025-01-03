cmake_minimum_required(VERSION 3.15)

project(HelloWorld LANGUAGES CXX)

add_executable(hello_world main.cpp)

target_compile_options(hello_world PRIVATE -Wall -Wextra -pedantic)

install(TARGETS hello_world
        RUNTIME DESTINATION bin)
