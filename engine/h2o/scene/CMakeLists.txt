cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

add_library(h2o_scene STATIC
    src/scene/actor.cpp
    src/scene/component.cpp
    src/scene/scene.cpp
    src/scene/scene_module.cpp
    src/scene/scene_networking_system.cpp
    src/scene/scene_system.cpp
    src/scene/transform.cpp

    src/scene_rendering/camera_component.cpp
    src/scene_rendering/mesh_renderer_component.cpp
    src/scene_rendering/rendering_scene_system.cpp
)

target_link_libraries(h2o_scene
    PUBLIC
        h2o_networking # Needed public because of networking_utils being used in scene_module.h

        bitsery
        EnTT::EnTT

    PRIVATE
        h2o_core
        h2o_rendering
        h2o_windowing
)

target_include_directories(h2o_scene
    PUBLIC
        include/

        h2o_networking

        bitsery
        EnTT::EnTT

    PRIVATE
        h2o_core
        h2o_rendering
        h2o_windowing
)