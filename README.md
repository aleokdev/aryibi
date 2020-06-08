# Aryibi
Aryibi is a C++17 graphics library primarily focused on 2D graphics such as sprites. This is mostly meant
for private projects and [arpiyi](https://github.com/alexdevteam/arpiyi), not for public use, but you are free
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