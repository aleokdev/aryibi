add_subdirectory(anton_math)
add_subdirectory(glfw)
add_subdirectory(glad)

add_library(imgui STATIC imgui/imgui_draw.cpp imgui/imgui_demo.cpp imgui/imgui_widgets.cpp imgui/imgui.cpp imgui/examples/imgui_impl_glfw.cpp imgui/examples/imgui_impl_opengl3.cpp)
target_include_directories(imgui PUBLIC imgui)
target_compile_definitions(imgui PUBLIC IMGUI_IMPL_OPENGL_LOADER_GLAD)
target_link_libraries(imgui PRIVATE glfw glad)

target_link_libraries(aryibi PUBLIC anton_math)
target_link_libraries(aryibi PRIVATE glfw glad imgui)
target_include_directories(aryibi PRIVATE stb)