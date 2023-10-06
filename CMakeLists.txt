cmake_minimum_required(VERSION 3.7...3.27)

if(${CMAKE_VERSION} VERSION_LESS 3.12)
  cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
endif()

project(CGameEngine VERSION 1.0
                    DESCRIPTION "Game engine in C"
                    LANGUAGES C CXX)

add_compile_options(-g -Wall -m64)
add_link_options(-g -Wall -m64)

set(RAYLIB_VERSION 4.5.0)
find_package(raylib ${RAYLIB_VERSION} QUIET) # QUIET or REQUIRED
if (NOT raylib_FOUND) # If there's none, fetch and build raylib
  include(FetchContent)
  FetchContent_Declare(
    raylib
    DOWNLOAD_EXTRACT_TIMESTAMP OFF
    URL https://github.com/raysan5/raylib/archive/refs/tags/${RAYLIB_VERSION}.tar.gz
  )
  FetchContent_GetProperties(raylib)
  if (NOT raylib_POPULATED) # Have we downloaded raylib yet?
    set(FETCHCONTENT_QUIET NO)
    FetchContent_Populate(raylib)
    set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE) # don't build the supplied examples
    add_subdirectory(${raylib_SOURCE_DIR} ${raylib_BINARY_DIR})
  endif()
endif()

aux_source_directory(lib/imgui IMGUI_SRC)
add_library(imgui ${IMGUI_SRC})
target_include_directories(imgui PRIVATE lib/ lib/imgui/ include/)

add_executable(cgame src/cgame.c src/arena.c src/console.c src/ecs.c src/event.c)
target_include_directories(cgame PUBLIC include)
target_link_directories(cgame PUBLIC lib)
target_link_libraries(cgame PUBLIC raylib winmm kernel32 opengl32 gdi32)

add_executable(editor src/editorimgui.c)
target_include_directories(editor PUBLIC include)
target_link_directories(editor PUBLIC lib)
target_link_libraries(editor PUBLIC raylib imgui stdc++ winmm kernel32 opengl32 gdi32)
