#pragma once

#include "block.h"
#include "core/handle_types.h"
#include "core/types.h"
#include "voxel/voxel_constants.h"

#include <iterator>
#include <set>
#include <vector>

namespace h2o
{
    class ChunkSystem;
    class VoxelModule;

    class Chunk
    {
    public:

        Chunk() = default;

        void init(const VoxelModule& voxel_module);
        void tick();

        [[nodiscard]] Block get_block_at(const v3i& local_pos) const;
        void set_block_at(const v3i& local_pos, Block block);

        [[nodiscard]] const v3i& chunk_pos() const { return m_chunk_pos; }
        [[nodiscard]] bool is_empty() const { return m_is_empty; }

        static constexpr bool is_valid_pos(const v3i& local_pos)
        {
            return
                local_pos.x >= 0 && local_pos.x < voxel_constants::chunk_size &&
                local_pos.y >= 0 && local_pos.y < voxel_constants::chunk_size &&
                local_pos.z >= 0 && local_pos.z < voxel_constants::chunk_size;
        }

        template<typename S>
        void serialize(S& s)
        {
            s(m_blocks, m_blocks_to_tick, m_chunk_pos, m_is_empty);
        }

    private:

        [[nodiscard]] bool is_initialized() const { return !m_blocks.empty(); }

    private:

        friend class ChunkColumn;

        std::vector<Block> m_blocks{};
        std::set<u32> m_blocks_to_tick{};

        v3i m_chunk_pos{};

        const VoxelModule* m_voxel_module { nullptr };

        bool m_is_empty { true };

    };
}