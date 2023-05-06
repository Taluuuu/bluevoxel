cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

add_library(h2o_voxel
    src/chunk.cpp
    src/chunk_mgr.cpp
    src/voxel_module.cpp
)

target_link_libraries(h2o_voxel
    PRIVATE
        h2o_core
)

target_include_directories(h2o_voxel
    PUBLIC
        include/

    PRIVATE
        h2o_core
)