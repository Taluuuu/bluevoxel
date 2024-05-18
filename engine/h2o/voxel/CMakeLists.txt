cmake_minimum_required(VERSION 3.20)

add_library(h2o_voxel
    src/block.cpp
    src/chunk_container_interface.cpp
    src/block_presets/block_preset_base.cpp
    src/block_presets/block_preset_crop.cpp
    src/chunk.cpp
    src/chunk_column.cpp
    src/chunk_generation_queue.cpp
    src/chunk_generators/chunk_generator_base.cpp
    src/chunk_generators/chunk_generator_flat.cpp
    src/chunk_generators/chunk_generator_sphere.cpp
    src/chunk_generators/chunk_generator_terrain.cpp
    src/chunk_manager.cpp
    src/chunk_manager_base.cpp
    src/chunk_region.cpp
    src/uncooked_block_model.cpp
    src/voxel_bounds.cpp
    src/voxel_module.cpp
    src/voxel_pack.cpp
    src/voxel_ray.cpp
)

set_target_properties(h2o_voxel
    PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED True
)

target_link_libraries(h2o_voxel
    PUBLIC
        FastNoise
        bitsery
        yaml-cpp

    PRIVATE
        h2o_core
        h2o_scene

        magic_enum
)

target_include_directories(h2o_voxel
    PUBLIC
        include/
        FastNoise
        bitsery
        yaml-cpp

    PRIVATE
        h2o_core
        h2o_scene

        magic_enum
)