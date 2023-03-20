cmake_minimum_required(VERSION 3.20)
project(h2o_rendering)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

add_library(h2o_scene
    src/scene_module.cpp
    src/scene.cpp
    src/actor.cpp
    src/component.cpp
    src/actors/fps_character_actor.cpp
    src/components/camera_component.cpp)

target_link_libraries(h2o_scene
    PRIVATE
        h2o_core)

target_include_directories(h2o_scene
    PUBLIC
        include/

    PRIVATE
        h2o_core)