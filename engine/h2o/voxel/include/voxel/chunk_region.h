#pragma once

#include <vector>

#include "chunk_view.h"
#include "structures/voxel_structure_manager.h"

namespace h2o
{
    class Chunk;
    class VoxelStructure;

    using ChunkRegionView = ChunkView<v3i{
        voxel_constants::chunk_region_size,
        voxel_constants::vertical_chunk_count,
        voxel_constants::chunk_region_size }>;

    class ChunkRegion
    {
    public:

        explicit ChunkRegion(v2i position);

        // Places the bit of contained structures that fits in this chunk
        void place_structures(Chunk& chunk) const;

        void generate_structures(ChunkRegionView& destination);

        enum class GenerationState
        { None, Terrain, Finished };

        [[nodiscard]] GenerationState generation_state() const { return m_generation_state; }

    private:

        std::vector<VoxelStructureInstance> m_structures{};
        v2i m_position{};

        GenerationState m_generation_state = GenerationState::None;

    };
}
