cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

add_library(h2o_voxel_server
    src/voxel_server_module.cpp
)

target_link_libraries(h2o_voxel_server
    PRIVATE
        h2o_core
        h2o_voxel
)

target_include_directories(h2o_voxel_server
    PUBLIC
        include/

    PRIVATE
        h2o_core
        h2o_voxel
)