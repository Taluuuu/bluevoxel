cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

add_library(h2o_scene
    src/actor.cpp
    src/component.cpp
    src/scene.cpp
    src/scene_module.cpp
    src/scene_networking_system.cpp
    src/scene_system.cpp
    src/transform.cpp
)

target_link_libraries(h2o_scene
    PRIVATE
        h2o_core
        h2o_networking
)

target_include_directories(h2o_scene
    PUBLIC
        include/

    PRIVATE
        h2o_core
        h2o_networking
)