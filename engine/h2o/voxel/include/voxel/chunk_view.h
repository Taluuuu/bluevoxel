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

    class ChunkView
    {
    public:

        explicit ChunkView(const v3i& corner, const v3i& view_size);

        void for_each_chunk(const std::function<void(Chunk&)>& function);
        void for_each_chunk(const std::function<void(const Chunk&)>& function) const;

        // Loop through all blocks that are not air
        void for_each_block(const std::function<void(const v3i&, const Block&)>& function) const;

        [[nodiscard]] std::optional<Block> get_block_at(const v3i& block_pos, ViewRelativeTo relative_to = ViewRelativeTo::World) const;
        bool set_block_at(const v3i& block_pos, Block block, ViewRelativeTo relative_to = ViewRelativeTo::World);

        [[nodiscard]] std::optional<std::tuple<Block, u8>> get_block_and_light_level_at(const v3i& block_pos, ViewRelativeTo relative_to = ViewRelativeTo::World) const;

        [[nodiscard]] Chunk* get_chunk_at(const v3i& relative_chunk_pos, ViewRelativeTo relative_to = ViewRelativeTo::World);
        [[nodiscard]] const Chunk* get_chunk_at(const v3i& relative_chunk_pos, ViewRelativeTo relative_to = ViewRelativeTo::World) const;

        [[nodiscard]] v3i center_chunk_pos() const { return m_corner + v3i{ m_view_size } / 2; }
        [[nodiscard]] v3i corner_chunk_pos() const { return m_corner; }
        [[nodiscard]] v3i size() const { return m_view_size; }
        [[nodiscard]] bool is_generated() const;

    private:

        friend class ChunkManager;
        // The chunk must be from the chunk column to ensure thread-safety (see comment below)
        void add_chunk(Chunk& chunk, const std::shared_ptr<ChunkColumnData>& chunk_column);

    private:

        [[nodiscard]] v3i get_relative_to_chunk_pos(ViewRelativeTo relative_to) const;
        [[nodiscard]] bool in_bounds(const v3i& local_chunk_pos) const;
        [[nodiscard]] size_t to_index(const v3i& local_chunk_pos) const;

    private:

        // 3D vector of chunk pointers
        std::vector<Chunk*> m_chunks{};

        // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        // !!! Make sure chunk columns stay allocated while chunk pointers are in use !!!
        // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
        std::unordered_set< std::shared_ptr<ChunkColumnData> > m_chunk_columns{};

        v3i m_corner, m_view_size{};

    };
}