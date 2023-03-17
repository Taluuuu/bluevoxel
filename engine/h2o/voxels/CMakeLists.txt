cmake_minimum_required(VERSION 3.20)
project(h2o_voxels)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

add_library(h2o_voxels
    src/voxels_module.cpp
)

target_link_libraries(h2o_voxels
    PRIVATE
        h2o_core
)

target_include_directories(h2o_voxels
    PUBLIC
        include/

    PRIVATE
        h2o_core
)