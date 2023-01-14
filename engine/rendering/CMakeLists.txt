cmake_minimum_required(VERSION 3.20)
project(rendering)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

add_library(rendering
    src/rendering_module.cpp
    src/vulkan/debug_messenger_vulkan.cpp
    src/vulkan/pipeline_vulkan.cpp
    src/vulkan/renderer_vulkan.cpp
    src/vulkan/shader_vulkan.cpp
    src/vulkan/swapchain_vulkan.cpp
)

target_link_libraries(rendering
    PRIVATE
        core
        windowing

        ${Vulkan_LIBRARIES}
        glfw
)

target_include_directories(rendering
    PUBLIC
        include/

    PRIVATE
        core
        windowing

        ${VULKAN_PATH}/Include/
        glfw
)