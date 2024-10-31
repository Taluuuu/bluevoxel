#pragma once

#include "chunk.h"
#include "chunk_column_data.h"
#include "voxel/block.h"
#include "voxel_utils.h"

#include <array>
#include <optional>

namespace h2o
{
    class Chunk;

    enum class ViewRelativeTo
    { ViewCenter, ViewCorner, World };

    template<v3i ViewSize>
    class ChunkView
    {
    public:

        static_assert(ViewSize.x > 0 && ViewSize.y > 0 && ViewSize.z > 0);

        explicit ChunkView(const v3i& corner);

        void for_each_chunk(const std::function<void(Chunk&)>& function);
        void for_each_chunk(const std::function<void(const Chunk&)>& function) const;

        // Loop through all blocks that are not air
        void for_each_block(const std::function<void(const v3i&, const Block&)>& function) const;

        [[nodiscard]] std::optional<Block> get_block_at(const v3i& block_pos, ViewRelativeTo relative_to = ViewRelativeTo::World) const;
        bool set_block_at(const v3i& block_pos, Block block, ViewRelativeTo relative_to = ViewRelativeTo::World);

        [[nodiscard]] std::optional<std::tuple<Block, u8>> get_block_and_light_level_at(const v3i& block_pos, ViewRelativeTo relative_to = ViewRelativeTo::World) const;

        [[nodiscard]] Chunk* get_chunk_at(const v3i& relative_chunk_pos, ViewRelativeTo relative_to = ViewRelativeTo::World);
        [[nodiscard]] const Chunk* get_chunk_at(const v3i& relative_chunk_pos, ViewRelativeTo relative_to = ViewRelativeTo::World) const;

        [[nodiscard]] v3i center_chunk_pos() const { return m_corner + v3i{ ViewSize } / 2; }
        [[nodiscard]] v3i corner_chunk_pos() const { return m_corner; }
        [[nodiscard]] static v3i size() { return ViewSize; }
        [[nodiscard]] bool is_generated() const;

    private:

        friend class ChunkManager;
        // The chunk must be from the chunk column to ensure thread-safety (see comment below)
        void add_chunk(Chunk& chunk, const std::shared_ptr<ChunkColumnData>& chunk_column);

    private:

        [[nodiscard]] v3i get_relative_to_chunk_pos(ViewRelativeTo relative_to) const;
        [[nodiscard]] static constexpr bool in_bounds(const v3i& local_chunk_pos);
        [[nodiscard]] static constexpr size_t to_index(const v3i& local_chunk_pos);

    private:

        // 3D vector of chunk pointers
        std::array<Chunk*, ViewSize.x * ViewSize.y * ViewSize.z> m_chunks{};

        // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        // !!! Make sure chunk columns stay allocated while chunk pointers are in use !!!
        // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        std::unordered_set< std::shared_ptr<ChunkColumnData> > m_chunk_columns{};

        v3i m_corner{};

    };

    template<v3i ViewSize>
    ChunkView<ViewSize>::ChunkView(const v3i& corner)
        : m_corner(corner)
    {}

    template<v3i ViewSize>
    void ChunkView<ViewSize>::for_each_chunk(const std::function<void(Chunk&)>& function)
    {
        for (Chunk* chunk : m_chunks)
        {
            if (chunk)
                function(*chunk);
        }
    }

    template<v3i ViewSize>
    void ChunkView<ViewSize>::for_each_chunk(const std::function<void(const Chunk&)>& function) const
    {
        for (const Chunk* chunk : m_chunks)
        {
            if (chunk)
                function(*chunk);
        }
    }

    template<v3i ViewSize>
    void ChunkView<ViewSize>::for_each_block(const std::function<void(const v3i&, const Block&)>& function) const
    {
        for (const Chunk* chunk : m_chunks)
        {
            if (!chunk || chunk->is_empty())
                continue;

            const v3i chunk_corner_pos = chunk->chunk_pos() * voxel_constants::chunk_size;
            for (i32 i = 0; i < voxel_constants::chunk_size; i++)
            for (i32 j = 0; j < voxel_constants::chunk_size; j++)
            for (i32 k = 0; k < voxel_constants::chunk_size; k++)
            {
                const v3i local_block_pos{ i, j, k };
                const v3i world_block_pos = chunk_corner_pos + local_block_pos;
                if (const auto block = chunk->get_block_at(local_block_pos); block != Block::Air)
                    function(world_block_pos, block);
            }
        }
    }

    template<v3i ViewSize>
    std::optional<Block> ChunkView<ViewSize>::get_block_at(const v3i& block_pos, ViewRelativeTo relative_to) const
    {
        if (const Chunk* chunk = get_chunk_at(voxel_utils::block_to_chunk_pos(block_pos), relative_to))
            return chunk->get_block_at(voxel_utils::block_pos_to_within_chunk(block_pos));

        return std::nullopt;
    }

    template<v3i ViewSize>
    bool ChunkView<ViewSize>::set_block_at(const v3i& block_pos, Block block, ViewRelativeTo relative_to)
    {
        if (Chunk* chunk = get_chunk_at(voxel_utils::block_to_chunk_pos(block_pos), relative_to))
        {
            chunk->set_block_at(voxel_utils::block_pos_to_within_chunk(block_pos), block);
            return true;
        }

        return false;
    }

    template<v3i ViewSize>
    std::optional<std::tuple<Block, u8>> ChunkView<ViewSize>::get_block_and_light_level_at(
        const v3i& block_pos,
        const ViewRelativeTo relative_to) const
    {
        if (const Chunk* chunk = get_chunk_at(voxel_utils::block_to_chunk_pos(block_pos), relative_to))
        {
            const v3i within_chunk = voxel_utils::block_pos_to_within_chunk(block_pos);
            return std::tuple {
                chunk->get_block_at(within_chunk),
                chunk->get_light_level_at(within_chunk)
            };
        }

        return std::nullopt;
    }

    template<v3i ViewSize>
    Chunk* ChunkView<ViewSize>::get_chunk_at(const v3i& relative_chunk_pos, ViewRelativeTo relative_to)
    {
        const v3i offset = get_relative_to_chunk_pos(relative_to) - m_corner;
        const v3i local_chunk_pos = relative_chunk_pos + offset;

        if (!in_bounds(local_chunk_pos))
            return nullptr;

        return m_chunks[to_index(local_chunk_pos)];
    }

    template<v3i ViewSize>
    const Chunk* ChunkView<ViewSize>::get_chunk_at(const v3i& relative_chunk_pos, ViewRelativeTo relative_to) const
    {
        const v3i offset = get_relative_to_chunk_pos(relative_to) - m_corner;
        const v3i local_chunk_pos = relative_chunk_pos + offset;

        if (!in_bounds(local_chunk_pos))
            return nullptr;

        return m_chunks[to_index(local_chunk_pos)];
    }

    template<v3i ViewSize>
    bool ChunkView<ViewSize>::is_generated() const
    {
        for (const auto& chunk : m_chunks)
        {
            if (!chunk || !chunk->is_generated())
                return false;
        }

        return true;
    }

    template<v3i ViewSize>
    void ChunkView<ViewSize>::add_chunk(Chunk& chunk, const std::shared_ptr<ChunkColumnData>& chunk_column)
    {
        const v3i local_chunk_pos = chunk.chunk_pos() - m_corner;
        assert(in_bounds(local_chunk_pos));

        m_chunks[to_index(local_chunk_pos)] = &chunk;
        m_chunk_columns.insert(chunk_column);
    }

    template<v3i ViewSize>
    v3i ChunkView<ViewSize>::get_relative_to_chunk_pos(ViewRelativeTo relative_to) const
    {
        switch (relative_to)
        {
        case ViewRelativeTo::ViewCenter:
            return center_chunk_pos();
        case ViewRelativeTo::ViewCorner:
            return m_corner;
        default:
        case ViewRelativeTo::World:
            return v3i{ 0 };
        }
    }

    template<v3i ViewSize>
    constexpr bool ChunkView<ViewSize>::in_bounds(const v3i& local_chunk_pos)
    {
        return
            local_chunk_pos.x >= 0 && local_chunk_pos.x < ViewSize.x &&
            local_chunk_pos.y >= 0 && local_chunk_pos.y < ViewSize.y &&
            local_chunk_pos.z >= 0 && local_chunk_pos.z < ViewSize.z;
    }

    template<v3i ViewSize>
    constexpr size_t ChunkView<ViewSize>::to_index(const v3i& local_chunk_pos)
    {
        assert(in_bounds(local_chunk_pos));

        return
            local_chunk_pos.z * ViewSize.x * ViewSize.y +
            local_chunk_pos.y * ViewSize.x +
            local_chunk_pos.x;
    }

    using ChunkColumnView = ChunkView<v3i{ 1, voxel_constants::vertical_chunk_count, 1 }>;
    using ChunkMeshingView = ChunkView<v3i{3}>;
}