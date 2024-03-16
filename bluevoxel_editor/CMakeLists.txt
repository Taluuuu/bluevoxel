cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

add_executable(bluevoxel_editor
    src/block_editor/block_editor_workspace.cpp
    src/block_editor/block_model_editor/block_model_editor.cpp
    src/block_editor/block_model_editor/triangle_selection.cpp
    src/block_editor/block_model_editor/uv_editor.cpp
    src/block_editor/block_model_editor/vertex_selection.cpp
    src/block_editor/block_type_editor/block_type_editor.cpp
    src/block_renderer.cpp
    src/bluevoxel_editor_module.cpp
    src/gizmo.cpp
    src/main.cpp
    src/selection_manager.cpp
    src/world_gen_editor/world_gen_editor_workspace.cpp
)

target_link_libraries(bluevoxel_editor
    PRIVATE
        h2o_core
        h2o_game_framework
        h2o_input
        h2o_networking
        h2o_physics
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
        h2o_physics
        h2o_scene
        h2o_scene_rendering
        h2o_ui
        h2o_voxel
        h2o_voxel_client
        h2o_voxel_rendering
        h2o_windowing
)