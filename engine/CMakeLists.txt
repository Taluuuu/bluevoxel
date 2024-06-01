cmake_minimum_required(VERSION 3.20)

# glfw
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(lib/glfw)

# OpenGL
add_subdirectory(lib/glad)

# ImGui
set(IMGUI_USE_OPENGL True)
set(IMGUI_USE_GLFW True)
add_subdirectory(lib/imgui)

# GameNetworkingSockets
find_package(GameNetworkingSockets REQUIRED)

# glm
add_subdirectory(lib/glm)

# magic_enum
add_subdirectory(lib/magic_enum)

# fmt
add_subdirectory(lib/fmt)

# Catch2
add_subdirectory(lib/Catch2)

# observable_unique_ptr
add_subdirectory(lib/observable_unique_ptr)

# stb
add_subdirectory(lib/stb)

# yaml-cpp
add_subdirectory(lib/yaml-cpp)

# bitsery
add_subdirectory(lib/bitsery)

# assimp
find_package(assimp REQUIRED)

# FastNoise2
add_subdirectory(lib/FastNoise2)

# Engine libraries
add_subdirectory(h2o/core)
add_subdirectory(h2o/input)
add_subdirectory(h2o/windowing)
add_subdirectory(h2o/rendering)
add_subdirectory(h2o/ui)
add_subdirectory(h2o/scene)
add_subdirectory(h2o/scene_rendering)
add_subdirectory(h2o/game_framework)
add_subdirectory(h2o/voxel)
add_subdirectory(h2o/networking)
add_subdirectory(h2o/physics)
add_subdirectory(h2o/inventory)