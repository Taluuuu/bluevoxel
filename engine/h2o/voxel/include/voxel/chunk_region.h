#pragma once

#include "structures/voxel_structure_manager.h"
#include "voxel_constants.h"

#include <vector>

namespace h2o
{
    class Chunk;
    class VoxelStructure;

    constexpr v3i ChunkRegionExtents{
        voxel_constants::chunk_region_size,
        voxel_constants::vertical_chunk_count,
        voxel_constants::chunk_region_size
    };

    class ChunkRegion
    {
    public:

        explicit ChunkRegion(v2i position);

        void register_structures(const std::vector<VoxelStructureInstance>& structures);
        [[nodiscard]] const auto& structures() const { return m_structures; }

        [[nodiscard]] v2i region_pos() const { return m_position; }

        enum class GenerationState
        { Pending, Terrain, Structures, Finished = Structures };

    public:

        GenerationState generation_state = GenerationState::Pending;

    private:

        std::vector<VoxelStructureInstance> m_structures{};
        v2i m_position{};

    };
}
