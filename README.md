# Aryibi
Aryibi is a C++17 graphics library primarily focused on 2D graphics such as sprites. This is mostly meant
for private projects and [arpiyi](https://github.com/alexdevteam/arpiyi), not for public use.

Core functionality:
- A fully functional generic renderer that supports simple lighting and indexed textures (Useful for
pixel art)
- A simple windowing/input interface with GLFW backend
- Utilities and tools for working with and loading different types of sprites, including autotiles
- [ImGui](https://github.com/ocornut/imgui/) integration (Provides an imgui_id() function for textures +
start_frame and finish_frame update the imgui frame)

Stuff to do:
- Remove imgui dependency. Create an api-agnostic UI wrapper.

The renderer's default implementation uses OpenGL, but if you are a Vulkan (or other API) nerd, you
are free to implement the interface with your own backend.