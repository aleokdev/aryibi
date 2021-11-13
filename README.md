
<div align="center">
    <img src="https://user-images.githubusercontent.com/16354904/141654563-c1243f3e-b8f1-4c6f-b23c-de4a9484070b.png" alt="Sample screenshot" title="Aryibi" width="600">
</div>

# Aryibi ![C++ Standard - 17](https://img.shields.io/static/v1?label=C%2B%2B+Standard&message=17&color=informational&logo=c%2B%2B)

Aryibi is a C++ graphics library primarily focused on 2D graphics such as sprites. This is mostly meant
for private projects and [arpiyi](https://github.com/alexdevteam/arpiyi), but you are free
to use or fork the library.

The core principle of aryibi is being generic without sacrificing simplicity. This means that **the backend
shouldn't matter at all**. When you want to use, for example, the renderer in other parts of your
application, you shouldn't care about internal API state or API-specific data, only the surface of it.

This repo contains an example project inside of it (Which will probably be moved to a separate repo later on)
which showcases most of the basic functionality of the library.

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

## Building
Aryibi provides all the libraries it needs as submodules (This will most likely change soon).
