cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

add_library(h2o_voxel_client
    src/voxel_client_module.cpp
)

target_link_libraries(h2o_voxel_client
    PRIVATE
        h2o_core
        h2o_networking
        h2o_voxel
)

target_include_directories(h2o_voxel_client
    PUBLIC
        include/

    PRIVATE
        h2o_core
        h2o_networking
        h2o_voxel
)