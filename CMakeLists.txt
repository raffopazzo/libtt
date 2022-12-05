cmake_minimum_required(VERSION 3.8)
project(
    libtt
    VERSION 0.0.1
    DESCRIPTION "C++ Library for Type Theory"
    LANGUAGES CXX
)

find_package(Boost REQUIRED)

add_subdirectory(lib)

enable_testing()
add_subdirectory(test)
