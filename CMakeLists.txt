cmake_minimum_required(VERSION 3.15)
project(aryibi LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)

add_library(aryibi STATIC src/renderer.cpp src/renderer_types.cpp)
target_include_directories(aryibi PUBLIC include)
target_include_directories(aryibi PRIVATE src)
add_subdirectory(lib)