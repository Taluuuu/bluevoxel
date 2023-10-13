cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

add_executable(bluevoxel_client
    src/bluevoxel_client_module.cpp
    src/main.cpp
)

target_link_libraries(bluevoxel_client
    PRIVATE
        h2o_core
        h2o_game_framework
        h2o_input
        h2o_networking
        h2o_rendering
        h2o_scene
        h2o_scene_rendering
        h2o_voxel
        h2o_voxel_client
        h2o_voxel_rendering
        h2o_windowing

        imgui
)

target_include_directories(bluevoxel_client
    PRIVATE
        h2o_core
        h2o_game_framework
        h2o_input
        h2o_networking
        h2o_rendering
        h2o_scene
        h2o_scene_rendering
        h2o_voxel
        h2o_voxel_client
        h2o_voxel_rendering
        h2o_windowing

        imgui
)

# Add this resource path
set(RESOURCE_PATHS ${RESOURCE_PATHS};${CMAKE_CURRENT_SOURCE_DIR}/res PARENT_SCOPE)