cmake_minimum_required(VERSION 3.15)
project(aryibi LANGUAGES C CXX)

set(ARYIBI_BACKEND "glfw-opengl" CACHE STRING "The backend to use. Can be: 'glfw-opengl', 'none'. Default: 'glfw-opengl'")

set(CMAKE_CXX_STANDARD 17)

add_library(aryibi STATIC src/sprites.cpp)

target_include_directories(aryibi PUBLIC include)
target_include_directories(aryibi PRIVATE src)

# ARYIBI_REQUIRED_LIBS are the required library targets that must be supplied externally.
if (ARYIBI_BACKEND STREQUAL "glfw-opengl")
    message(STATUS "[aryibi] Using GLFW + OpenGL backend")
    message(STATUS "[aryibi] For the stb required library target, create an add_custom_target with an interface
    target_include_directories containing the stb directory.")
    message(STATUS "[aryibi] For the imgui required library target, create an static library that includes the imgui
    directory publicly and has the following sources: imgui/imgui_draw.cpp imgui/imgui_demo.cpp imgui/imgui_widgets.cpp
    imgui/imgui.cpp imgui/examples/imgui_impl_glfw.cpp imgui/examples/imgui_impl_opengl3.cpp")
    target_sources(aryibi PRIVATE src/renderer/opengl/renderer.cpp src/renderer/opengl/renderer_types.cpp
            src/windowing/glfw/windowing.cpp)
    set(ARYIBI_REQUIRED_LIBS glad glfw imgui stb)
elseif (ARYIBI_BACKEND STREQUAL "none")
else ()
    message(FATAL_ERROR "Please select a valid backend for ARYIBI_BACKEND.")
endif ()

foreach (REQUIRED_LIB ${ARYIBI_REQUIRED_LIBS})
    if (NOT TARGET ${REQUIRED_LIB})
        message(FATAL_ERROR "You must supply the '${REQUIRED_LIB}' library target before including aryibi in order to\
        meet the requirements of the ${ARYIBI_BACKEND} aryibi backend.")
    endif ()
    target_link_libraries(aryibi PRIVATE ${REQUIRED_LIB})
endforeach ()

add_subdirectory(lib)
add_subdirectory(assets)
add_subdirectory(example)