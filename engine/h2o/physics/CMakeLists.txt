cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

add_library(h2o_physics STATIC
    src/collider_aabb.cpp
    src/math_helpers.cpp
    src/physics_module.cpp
    src/ray_intersections.cpp
    src/scene/collider_component.cpp
    src/scene/physics_system.cpp
)

target_link_libraries(h2o_physics
    PRIVATE
        h2o_core
        h2o_scene
)

target_include_directories(h2o_physics
    PUBLIC
        include/

    PRIVATE
        h2o_core
        h2o_scene
)