cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

add_executable(bluevoxel_server
    src/bluevoxel_server_module.cpp
    src/main.cpp
)

target_link_libraries(bluevoxel_server
    PRIVATE
        h2o_core
        h2o_graph
        h2o_networking
        h2o_scene
        h2o_voxel
        h2o_weather
)

target_include_directories(bluevoxel_server
    PUBLIC
        include/

    PRIVATE
        h2o_core
        h2o_graph
        h2o_networking
        h2o_scene
        h2o_voxel
        h2o_weather
)