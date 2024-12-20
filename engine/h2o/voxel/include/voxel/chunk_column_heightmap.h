#pragma once

#include "voxel_constants.h"

#include <array>

namespace h2o
{
    class Chunk;

    class ChunkColumnHeightmap
    {
    public:

        ChunkColumnHeightmap() = default;
        ~ChunkColumnHeightmap() = default;

        void update(const Chunk& chunk, const v3i& block_pos);
        [[nodiscard]] u32 get_height(v2i block_pos) const;

    private:

        using ChunkHeightmap = std::array<u8, voxel_constants::chunk_area>;
        std::array<ChunkHeightmap, voxel_constants::vertical_chunk_count> m_chunk_heightmaps{};

        std::array<u32, voxel_constants::chunk_area> m_column_heightmap{};

    };
}
