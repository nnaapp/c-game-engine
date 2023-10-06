cmake_minimum_required(VERSION 3.7...3.27)

if(${CMAKE_VERSION} VERSION_LESS 3.12)
  cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
endif()

project(CGameEngine VERSION 1.0
                    DESCRIPTION "Game engine in C"
                    LANGUAGES C)

add_compile_options(-g -Wall -m64)
add_link_options(-g -Wall -m64)

add_executable(cgame src/cgame.c src/arena.c src/console.c src/ecs.c src/event.c)
target_include_directories(cgame PUBLIC include)
target_link_directories(cgame PUBLIC lib)
target_link_libraries(cgame PUBLIC raylib winmm kernel32 opengl32 gdi32)

add_executable(editor src/editorimgui.c)
target_include_directories(editor PUBLIC include)
target_link_directories(editor PUBLIC lib)
target_link_libraries(editor PUBLIC raylib imgui stdc++ winmm kernel32 opengl32 gdi32)
