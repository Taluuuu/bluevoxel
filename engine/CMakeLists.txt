cmake_minimum_required(VERSION 3.20)

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

set(TNT_USE_GLFW True)
set(TNT_USE_OPENGL True)
set(TNT_USE_VULKAN False)

if(TNT_USE_GLFW)
    add_definitions(-DTNT_USE_GLFW)
endif()

if(TNT_USE_OPENGL)
    add_definitions(-DTNT_USE_OPENGL)
endif()

if(TNT_USE_VULKAN)
    add_definitions(-DTNT_USE_VULKAN)
endif()

# Engine libraries
add_subdirectory(h2o/core)
add_subdirectory(h2o/input)
add_subdirectory(h2o/windowing)
add_subdirectory(h2o/rendering)
add_subdirectory(h2o/scene)
add_subdirectory(h2o/voxels)

# Add this resource path
set(RESOURCE_PATHS ${RESOURCE_PATHS};${CMAKE_CURRENT_SOURCE_DIR}/res PARENT_SCOPE)