cmake_minimum_required(VERSION 3.20)
project(voxels)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

add_library(voxels
    src/voxels_module.cpp
)

target_link_libraries(voxels
    PRIVATE
        core
)

target_include_directories(voxels
    PUBLIC
        include/

    PRIVATE
        core
)