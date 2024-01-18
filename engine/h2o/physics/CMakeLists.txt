cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

add_library(h2o_physics
    src/math_helpers.cpp
    src/physics_module.cpp
    src/ray_intersections.cpp
)

target_link_libraries(h2o_physics
    PRIVATE
        h2o_core
)

target_include_directories(h2o_physics
    PUBLIC
        include/

    PRIVATE
        h2o_core
)