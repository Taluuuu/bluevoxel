cmake_minimum_required(VERSION 3.18)

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

# Engine libraries
add_subdirectory(core)
add_subdirectory(windowing)
add_subdirectory(rendering)