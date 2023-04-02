cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

add_library(h2o_scene_rendering
    src/scene_rendering_module.cpp
    src/camera_component.cpp
    src/mesh_renderer_component.cpp
    src/rendering_scene_system.cpp)

target_link_libraries(h2o_scene_rendering
    PRIVATE
        h2o_core
        h2o_scene
        h2o_rendering
        h2o_windowing)

target_include_directories(h2o_scene_rendering
    PUBLIC
        include/

    PRIVATE
        h2o_core
        h2o_scene
        h2o_rendering
        h2o_windowing)