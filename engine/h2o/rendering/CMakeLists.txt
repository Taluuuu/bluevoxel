cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

add_library(h2o_rendering STATIC
    src/buffer.cpp
    src/camera.cpp
    src/mesh.cpp
    src/pipeline.cpp
    src/renderer.cpp
    src/renderer_base.cpp
    src/rendering_module.cpp
    src/texture.cpp
    src/texture_array.cpp
    src/vertex_array.cpp

    src/opengl/pipeline_opengl.cpp
    src/opengl/renderer_opengl.cpp
)

target_link_libraries(h2o_rendering
    PUBLIC
        imgui

    PRIVATE
        h2o_core
        h2o_windowing

        assimp::assimp
        glfw
        stb
        glad
)

target_include_directories(h2o_rendering
    PUBLIC
        include/

        imgui

    PRIVATE
        h2o_core
        h2o_windowing

        assimp::assimp
        glfw
        stb
        glad
)