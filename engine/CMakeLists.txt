cmake_minimum_required(VERSION 3.20)

set(H2O_USE_GLFW True)
set(H2O_USE_OPENGL True)
set(H2O_USE_VULKAN False)

if(H2O_USE_GLFW)
    add_definitions(-DH2O_USE_GLFW)
endif()

if(H2O_USE_OPENGL)
    add_definitions(-DH2O_USE_OPENGL)
endif()

if(H2O_USE_VULKAN)
    add_definitions(-DH2O_USE_VULKAN)
endif()

# glfw
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(lib/glfw)

# glm
add_subdirectory(lib/glm)

# Vulkan
find_package(Vulkan REQUIRED)
set(VULKAN_PATH ${Vulkan_INCLUDE_DIRS})
STRING(REGEX REPLACE "/Include" "" VULKAN_PATH ${VULKAN_PATH})

# OpenGL
add_subdirectory(lib/glad)

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

# GameNetworkingSockets
find_package(GameNetworkingSockets REQUIRED)

# bitsery
add_subdirectory(lib/bitsery)

# assimp
find_package(assimp REQUIRED)

# FastNoise2
add_subdirectory(lib/FastNoise2)

# ImGui
set(IMGUI_USE_OPENGL ${H2O_USE_OPENGL})
set(IMGUI_USE_GLFW ${H2O_USE_GLFW})
add_subdirectory(lib/imgui)

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
add_subdirectory(h2o/voxel_rendering)
add_subdirectory(h2o/voxel_client)
add_subdirectory(h2o/voxel_server)
add_subdirectory(h2o/networking)
add_subdirectory(h2o/physics)