cmake_minimum_required(VERSION 3.20)
project(rendering)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

set(RENDERING_SOURCES
    src/rendering_module.cpp
    src/pipeline.cpp)

if(TNT_USE_VULKAN)
    list(APPEND RENDERING_SOURCES
        src/vulkan/debug_messenger_vulkan.cpp
        src/vulkan/device_vulkan.cpp
        src/vulkan/images/image_array_vulkan.cpp
        src/vulkan/images/image_view_vulkan.cpp
        src/vulkan/images/image_vulkan.cpp
        src/vulkan/pipeline_vulkan.cpp
        src/vulkan/renderer_vulkan.cpp
        src/vulkan/shader_vulkan.cpp
        src/vulkan/surface_vulkan.cpp
        src/vulkan/swapchain_vulkan.cpp)
endif()

if(TNT_USE_OPENGL)
    list(APPEND RENDERING_SOURCES
        src/opengl/renderer_opengl.cpp)
endif()

add_library(rendering ${RENDERING_SOURCES})

target_link_libraries(rendering
    PRIVATE
        core
        windowing

        ${Vulkan_LIBRARIES}
        glfw)

if (TNT_USE_OPENGL)
    target_link_libraries(rendering
        PRIVATE
            glad)
endif()

target_include_directories(rendering
    PUBLIC
        include/

    PRIVATE
        core
        windowing

        ${VULKAN_PATH}/Include/
        glfw)

if (TNT_USE_OPENGL)
    target_include_directories(rendering
        PRIVATE
            glad)
endif()