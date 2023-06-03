cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

set(RENDERING_SOURCES
    src/rendering_module.cpp
    src/pipeline.cpp
    src/camera.cpp)

if(H2O_USE_VULKAN)
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

if(H2O_USE_OPENGL)
    list(APPEND RENDERING_SOURCES
        src/opengl/buffer_opengl.cpp
        src/opengl/pipeline_opengl.cpp
        src/opengl/renderer_opengl.cpp
        src/opengl/vertex_array_opengl.cpp
        src/opengl/texture_opengl.cpp
        src/opengl/texture_array_opengl.cpp)
endif()

add_library(h2o_rendering ${RENDERING_SOURCES})

target_link_libraries(h2o_rendering
    PRIVATE
        h2o_core
        h2o_windowing

        ${Vulkan_LIBRARIES}
        glfw)

if (H2O_USE_OPENGL)
    target_link_libraries(h2o_rendering
        PRIVATE
            glad
            stb
            imgui)
endif()

target_include_directories(h2o_rendering
    PUBLIC
        include/

    PRIVATE
        h2o_core
        h2o_windowing

        ${VULKAN_PATH}/Include/
        glfw
        stb
        imgui)

if (H2O_USE_OPENGL)
    target_include_directories(h2o_rendering
        PRIVATE
            glad)
endif()