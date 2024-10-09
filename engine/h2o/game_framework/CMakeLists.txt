cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

add_library(h2o_game_framework STATIC
    src/game_framework_module.cpp
    src/actors/fps_character_actor.cpp
    src/components/fps_camera_component.cpp
    src/components/third_person_camera_component.cpp
)

target_link_libraries(h2o_game_framework
    PRIVATE
        h2o_core
        h2o_input
        h2o_physics
        h2o_scene
        h2o_rendering
)

target_include_directories(h2o_game_framework
    PUBLIC
        include/

    PRIVATE
        h2o_core
        h2o_input
        h2o_physics
        h2o_scene
        h2o_rendering
)