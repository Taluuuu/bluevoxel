cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

add_library(h2o_voxel
    src/block.cpp
    src/chunk.cpp
    src/chunk_system.cpp
    src/voxel_module.cpp
)

target_link_libraries(h2o_voxel
    PRIVATE
        magic_enum
        h2o_core
        h2o_scene
)

target_include_directories(h2o_voxel
    PUBLIC
        include/

    PRIVATE
        magic_enum
        h2o_core
        h2o_scene
)