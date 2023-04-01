cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

add_library(h2o_scene
    src/scene_module.cpp
    src/scene.cpp
    src/actor.cpp
    src/scene_system.cpp
    src/component.cpp
    src/transform.cpp)

target_link_libraries(h2o_scene
    PRIVATE
        h2o_core)

target_include_directories(h2o_scene
    PUBLIC
        include/

    PRIVATE
        h2o_core)