cmake_minimum_required(VERSION 3.20)

file(GLOB_RECURSE VOXEL_SOURCES "src/*.cpp")

add_library(h2o_voxel STATIC
    ${VOXEL_SOURCES}
)

set_target_properties(h2o_voxel
    PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED TRUE
)

target_link_libraries(h2o_voxel
    PUBLIC
        FastNoise
        bitsery
        yaml-cpp

    PRIVATE
        h2o_core
        h2o_input
        h2o_inventory
        h2o_networking
        h2o_physics
        h2o_rendering
        h2o_ui
        h2o_scene

        magic_enum
)

target_include_directories(h2o_voxel
    PUBLIC
        include/

        FastNoise
        bitsery
        yaml-cpp

    PRIVATE
        h2o_core
        h2o_input
        h2o_inventory
        h2o_networking
        h2o_physics
        h2o_rendering
        h2o_ui
        h2o_scene

        magic_enum
)