WIP toy game engine written in C using Raylib and rlImGui/cImGui.

# Requirements

Raylib 4.5+

rlImGui

Dear ImGui

cImGui

Windows or Wine (windows libraries are used, will be ported to Linux eventually)

Using CMake will build these for you, the ImGui package is included as source due to not playing well with FetchContent.

GLFW (used by raylib) can be cross-compiled using the .cmake files [here](https://github.com/glfw/glfw/tree/master/CMake),
as well as the corresponding cross-compiler (```-DCMAKE_C_COMPILER```), and the CMake argument ```-DCMAKE_TOOLCHAIN_FILE```.
