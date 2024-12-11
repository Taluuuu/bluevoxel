#pragma once

#include "block.h"
#include "core/handle_types.h"
#include "core/types.h"
#include "grid/grid_view_3d.h"
#include "voxel/direction.h"
#include "voxel/voxel_constants.h"
#include "voxel/voxel_module.h"

#include <iterator>
#include <shared_mutex>
#include <unordered_set>
#include <vector>

namespace h2o
{
    class VoxelModule;
    struct VoxelStructureInstance;

    class Chunk : public IGrid3DCell
    {
    public:

        Chunk() = default;

        class ViewType : public View<Chunk>
        {
        public:

            ViewType(const v3i& view_min, const v3i& view_size)
                : View(view_min, view_size) {}

        };

        // IGrid3DCell interface
        void init(const v3i& position) override;

        void tick();

        [[nodiscard]] Block get_block_at(const v3i& local_pos) const;
        void set_block_at(const v3i& local_pos, Block block);

        void place_structure(const VoxelStructureInstance& structure_instance);

        [[nodiscard]] const v3i& chunk_pos() const { return m_chunk_pos; }
        [[nodiscard]] bool is_empty() const { return m_is_empty; }

        // Indexed access
        [[nodiscard]] Block get_block_at(size_t index) const;
        void set_block_at(size_t index, Block block);

        [[nodiscard]] bool is_generated() const { return m_is_generated; }
        void mark_generated() { m_is_generated = true; }

        static constexpr size_t to_index(const v3i& local_pos);
        static constexpr bool is_valid_pos(const v3i& local_pos);
        static constexpr v3i to_block_pos(size_t index);

    private:

        [[nodiscard]] bool is_initialized() const { return !m_blocks.empty(); }

    private:

        std::vector<Block> m_blocks{};

        // Stores the indices of blocks to tick
        std::unordered_set<u32> m_blocks_to_tick{};

        v3i m_chunk_pos{};

        const VoxelModule* m_voxel_module = nullptr;

        bool m_is_empty = true;
        bool m_is_generated = false;

    };

    constexpr size_t Chunk::to_index(const v3i& local_pos)
    {
        return
            local_pos.y * voxel_constants::chunk_area +
            local_pos.x * voxel_constants::chunk_size +
            local_pos.z;
    }

    constexpr bool Chunk::is_valid_pos(const v3i& local_pos)
    {
        return
            local_pos.x >= 0 && local_pos.x < voxel_constants::chunk_size &&
            local_pos.y >= 0 && local_pos.y < voxel_constants::chunk_size &&
            local_pos.z >= 0 && local_pos.z < voxel_constants::chunk_size;
    }

    constexpr v3i Chunk::to_block_pos(size_t index)
    {
        v3i local_pos;

        local_pos.y = index / voxel_constants::chunk_area;
        index %= voxel_constants::chunk_area;

        local_pos.x = index / voxel_constants::chunk_size;
        local_pos.z = index % voxel_constants::chunk_size;

        return local_pos;
    }
}
