cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

add_library(h2o_voxel_rendering
    src/chunk_mesh.cpp
    src/chunk_rendering_system.cpp
    src/voxel_rendering_module.cpp
)

target_link_libraries(h2o_voxel_rendering
    PRIVATE
        h2o_core
        h2o_rendering
        h2o_scene
        h2o_voxel
        yaml-cpp
)

target_include_directories(h2o_voxel_rendering
    PUBLIC
        include/

    PRIVATE
        h2o_core
        h2o_rendering
        h2o_scene
        h2o_voxel
        yaml-cpp
)