#pragma once

#include "chunk.h"
#include "voxel/block.h"
#include "voxel_utils.h"

#include <array>
#include <optional>

namespace h2o
{
    class Chunk;

    enum class ViewRelativeTo
    { ViewCenter, ViewCorner, World };

    template<v3u ViewSize>
    class ChunkView
    {
    public:

        static_assert(ViewSize.x > 0 && ViewSize.y > 0 && ViewSize.z > 0);

        explicit ChunkView(const v3i& corner);

        [[nodiscard]] std::optional<Block> get_block_at(const v3i& block_pos, ViewRelativeTo relative_to = ViewRelativeTo::World) const;
        bool set_block_at(const v3i& block_pos, Block block, ViewRelativeTo relative_to = ViewRelativeTo::World);

        [[nodiscard]] Chunk* get_chunk_at(const v3i& relative_chunk_pos, ViewRelativeTo relative_to = ViewRelativeTo::World);
        [[nodiscard]] const Chunk* get_chunk_at(const v3i& relative_chunk_pos, ViewRelativeTo relative_to = ViewRelativeTo::World) const;

        [[nodiscard]] v3i center_chunk_pos() const;

        // private:
        //
        //     friend class ChunkManager_Base;
        void add_chunk(Chunk& chunk);

    private:

        [[nodiscard]] v3i get_relative_to_chunk_pos(ViewRelativeTo relative_to) const;
        [[nodiscard]] static constexpr bool in_range(const v3i& local_chunk_pos);
        [[nodiscard]] static constexpr size_t to_index(const v3i& local_chunk_pos);

    private:

        // 3D vector of chunk pointers
        std::array<Chunk*, ViewSize.x * ViewSize.y * ViewSize.z> m_chunks{};

        v3i m_corner{};

    };

    template<v3u ViewSize>
    ChunkView<ViewSize>::ChunkView(const v3i& corner)
        : m_corner(corner)
    {}

    template<v3u ViewSize>
    std::optional<Block> ChunkView<ViewSize>::get_block_at(const v3i& block_pos, ViewRelativeTo relative_to) const
    {
        if (const Chunk* chunk = get_chunk_at(voxel_utils::block_to_chunk_pos(block_pos), relative_to))
            return chunk->get_block_at(voxel_utils::block_pos_to_within_chunk(block_pos));

        return std::nullopt;
    }

    template<v3u ViewSize>
    bool ChunkView<ViewSize>::set_block_at(const v3i& block_pos, Block block, ViewRelativeTo relative_to)
    {
        if (Chunk* chunk = get_chunk_at(voxel_utils::block_to_chunk_pos(block_pos), relative_to))
        {
            chunk->set_block_at(voxel_utils::block_pos_to_within_chunk(block_pos), block);
            return true;
        }

        return false;
    }

    template<v3u ViewSize>
    Chunk* ChunkView<ViewSize>::get_chunk_at(const v3i& relative_chunk_pos, ViewRelativeTo relative_to)
    {
        const v3i offset = get_relative_to_chunk_pos(relative_to) - m_corner;
        const v3i local_chunk_pos = relative_chunk_pos + offset;

        if (!in_range(local_chunk_pos))
            return nullptr;

        return m_chunks[to_index(local_chunk_pos)];
    }

    template<v3u ViewSize>
    const Chunk* ChunkView<ViewSize>::get_chunk_at(const v3i& relative_chunk_pos, ViewRelativeTo relative_to) const
    {
        const v3i offset = get_relative_to_chunk_pos(relative_to) - m_corner;
        const v3i local_chunk_pos = relative_chunk_pos + offset;

        if (!in_range(local_chunk_pos))
            return nullptr;

        return m_chunks[to_index(local_chunk_pos)];
    }

    template<v3u ViewSize>
    v3i ChunkView<ViewSize>::center_chunk_pos() const
    {
        return m_corner + v3i{ ViewSize } / 2;
    }

    template<v3u ViewSize>
    void ChunkView<ViewSize>::add_chunk(Chunk& chunk)
    {
        const v3i local_chunk_pos = chunk.chunk_pos() - m_corner;
        assert(in_range(local_chunk_pos));

        m_chunks[to_index(local_chunk_pos)] = &chunk;
    }

    template<v3u ViewSize>
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

    template<v3u ViewSize>
    constexpr bool ChunkView<ViewSize>::in_range(const v3i& local_chunk_pos)
    {
        return
            local_chunk_pos.x >= 0 && local_chunk_pos.x < ViewSize.x &&
            local_chunk_pos.y >= 0 && local_chunk_pos.y < ViewSize.y &&
            local_chunk_pos.z >= 0 && local_chunk_pos.z < ViewSize.z;
    }

    template <v3u ViewSize>
    constexpr size_t ChunkView<ViewSize>::to_index(const v3i& local_chunk_pos)
    {
        assert(in_range(local_chunk_pos));

        return
            local_chunk_pos.z * ViewSize.x * ViewSize.y +
            local_chunk_pos.y * ViewSize.x +
            local_chunk_pos.x;
    }

    using ChunkColumnView = ChunkView<{ 1, voxel_constants::vertical_chunk_count, 1 }>;
}