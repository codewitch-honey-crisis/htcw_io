cmake_minimum_required(VERSION 3.24)
project(htcw_io VERSION 1.0)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_STATIC_LIBRARY_PREFIX "")
set(CMAKE_SHARED_LIBRARY_PREFIX "")

# get dependecies
include(FetchContent)
FetchContent_Declare(htcw_bits
    GIT_REPOSITORY https://github.com/codewitch-honey-crisis/htcw_bits.git
    GIT_TAG 1.0.14
    GIT_SHALLOW ON
    FIND_PACKAGE_ARGS 1.0.14
)

FetchContent_MakeAvailable(htcw_bits)

add_library(htcw_io src/io_stream.cpp)
target_link_libraries(htcw_io htcw_bits)
target_include_directories(htcw_io PUBLIC
    "${PROJECT_SOURCE_DIR}/include"
    "${PROJECT_BINARY_DIR}"
)
