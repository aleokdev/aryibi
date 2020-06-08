cmake_minimum_required(VERSION 3.15)
project(aryibi LANGUAGES C CXX)

set(ARYIBI_BACKEND "glfw-opengl" CACHE STRING "The backend to use. Can be: 'glfw-opengl', 'none'. Default: 'glfw-opengl'")

set(CMAKE_CXX_STANDARD 17)

add_library(aryibi STATIC src/sprites.cpp)

target_include_directories(aryibi PUBLIC include)
target_include_directories(aryibi PRIVATE src)

if (ARYIBI_BACKEND STREQUAL "glfw-opengl")
    message(STATUS "[aryibi] Using GLFW + OpenGL backend")
    target_sources(aryibi PRIVATE src/renderer/opengl/renderer.cpp src/renderer/opengl/renderer_types.cpp src/windowing/glfw/windowing.cpp)
elseif(ARYIBI_BACKEND STREQUAL "none")
else ()
    message(FATAL_ERROR "Please select a valid backend for ARYIBI_BACKEND.")
endif ()

add_subdirectory(lib)
add_subdirectory(assets)
add_subdirectory(example)