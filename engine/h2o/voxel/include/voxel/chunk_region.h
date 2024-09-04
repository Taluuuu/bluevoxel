#pragma once

#include "chunk_view.h"
#include "structures/voxel_structure_manager.h"

#include <vector>

namespace h2o
{
    class Chunk;
    class VoxelStructure;

    constexpr v3u ChunkRegionExtents{
        voxel_constants::chunk_region_size,
        voxel_constants::vertical_chunk_count,
        voxel_constants::chunk_region_size
    };

    using ChunkRegionView = ChunkView<ChunkRegionExtents>;

    class ChunkRegion
    {
    public:

        explicit ChunkRegion(v2i position);

        void register_structures(const std::vector<VoxelStructureInstance>& structures);

        // Places the bit of contained structures that fits in this chunk
        void place_structures(Chunk& chunk) const;

        [[nodiscard]] bool is_generated() const { return m_is_generated; }

    private:

        std::vector<VoxelStructureInstance> m_structures{};
        v2i m_position{};

        bool m_is_generated = false;

    };
}
