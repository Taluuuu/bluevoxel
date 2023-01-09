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

# magic_enum
add_subdirectory(lib/magic_enum)

# Engine libraries
add_subdirectory(core)
add_subdirectory(windowing)
add_subdirectory(rendering)

# Add this resource path
set(RESOURCE_PATHS ${RESOURCE_PATHS};${CMAKE_CURRENT_SOURCE_DIR}/res PARENT_SCOPE)