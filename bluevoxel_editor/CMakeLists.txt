cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

add_executable(bluevoxel_editor
    src/block_renderer.cpp
    src/bluevoxel_editor_module.cpp
    src/editor_modes/block_editor_workspace.cpp
    src/editor_modes/block_model_editor.cpp
    src/editor_modes/block_type_editor.cpp
    src/main.cpp
)

target_link_libraries(bluevoxel_editor
    PRIVATE
        h2o_core
        h2o_game_framework
        h2o_input
        h2o_networking
        h2o_rendering
        h2o_scene
        h2o_scene_rendering
        h2o_ui
        h2o_voxel
        h2o_voxel_rendering
        h2o_windowing
)

target_include_directories(bluevoxel_editor
    PRIVATE
        src/

        h2o_core
        h2o_game_framework
        h2o_input
        h2o_networking
        h2o_rendering
        h2o_scene
        h2o_scene_rendering
        h2o_ui
        h2o_voxel
        h2o_voxel_client
        h2o_voxel_rendering
        h2o_windowing
)