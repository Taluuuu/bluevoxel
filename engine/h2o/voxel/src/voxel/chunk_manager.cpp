#include "voxel/chunk_manager.h"

#include "core/engine.h"
#include "voxel/chunk_view.h"
#include "voxel/voxel_utils.h"

namespace h2o
{
    std::optional<Block> ChunkManager::get_block_at(const v3i& block_pos) const
    {
        std::optional<Block> block = std::nullopt;
        fetch_chunk(voxel_utils::block_to_chunk_pos(block_pos),
            [&](const Chunk* chunk)
            {
                if (chunk)
                    block = chunk->get_block_at(voxel_utils::block_pos_to_within_chunk(block_pos));
            }
        );

        return block;
    }

    bool ChunkManager::set_block_at(const v3i& block_pos, Block block)
    {
        bool success = false;
        fetch_chunk(voxel_utils::block_to_chunk_pos(block_pos),
            [&](Chunk* chunk)
            {
                if (chunk)
                {
                    chunk->set_block_at(voxel_utils::block_pos_to_within_chunk(block_pos), block);
                    success = true;
                }
            }
        );

        return success;
    }

    bool ChunkManager::chunk_exists(const v3i& chunk_pos) const
    {
        if (chunk_pos.y < 0 || chunk_pos.y >= voxel_constants::vertical_chunk_count)
            return false;

        return find_chunk_column({ chunk_pos.x, chunk_pos.z }) != nullptr;
    }

    void ChunkManager::fetch_or_create_chunk(const v3i& chunk_pos, const std::function<void(Chunk*)>& function)
    {
        view_or_create<v3i{1}>(chunk_pos,
            [&](ChunkView<v3i{1}>& view)
            {
                function(view.get_chunk_at(chunk_pos));
            }
        );
    }

    void ChunkManager::fetch_chunk(const v3i& chunk_pos, const std::function<void(Chunk*)>& function)
    {
        view<v3i{1}>(chunk_pos,
            [&](ChunkView<v3i{1}>& view)
            {
                function(view.get_chunk_at(chunk_pos));
            }
        );
    }

    void ChunkManager::fetch_chunk(const v3i& chunk_pos, const std::function<void(const Chunk*)>& function) const
    {
        view<v3i{1}>(chunk_pos,
            [&](const ChunkView<v3i{1}>& view)
            {
                function(view.get_chunk_at(chunk_pos));
            }
        );
    }

    void ChunkManager::view_or_create_chunk_column(
        v2i chunk_column_pos,
        const std::function<void(ChunkColumnView&)>& function)
    {
        view_or_create({ chunk_column_pos.x, 0, chunk_column_pos.y }, function);
    }

    void ChunkManager::view_chunk_column(
        v2i chunk_column_pos,
        const std::function<void(ChunkColumnView&)>& function)
    {
        view({ chunk_column_pos.x, 0, chunk_column_pos.y }, function);
    }

    void ChunkManager::view_chunk_column(
        v2i chunk_column_pos,
        const std::function<void(const ChunkColumnView&)>& function) const
    {
        view({ chunk_column_pos.x, 0, chunk_column_pos.y }, function);
    }

    void ChunkManager::view_for_meshing(
        const v3i& chunk_pos,
        const std::function<void(const ChunkMeshingView&)>& function) const
    {
        std::vector< std::unique_lock<std::shared_mutex> > chunk_locks{};
        chunk_locks.reserve(3 * 3 * 3);

        const v3i corner = chunk_pos - v3i{1};
        ChunkMeshingView chunk_view(corner);

        const auto add_chunk =
            [&](v3i offset)
            {
                const v3i neighbour_pos = chunk_pos + offset;
                if (!is_valid_chunk_y(neighbour_pos.y))
                    return;

                const v2i neighbour_col_pos{ neighbour_pos.x, neighbour_pos.z };
                if (const auto neighbour_col = find_chunk_column(neighbour_col_pos))
                {
                    auto& [chunk, mutex] = (*neighbour_col)[neighbour_pos.y];
                    chunk_view.add_chunk(chunk);
                    chunk_locks.emplace_back(mutex);
                }
            };

        // Order is important - XZY
        add_chunk({-1, 0, 0});
        add_chunk({ 0, 0,-1});
        add_chunk({ 0,-1, 0});
        add_chunk({ 0, 0, 0});
        add_chunk({ 0, 1, 0});
        add_chunk({ 0, 0, 1});
        add_chunk({ 1, 0, 0});

        function(chunk_view);
    }

    void ChunkManager::broadcast_events()
    {
        {
            std::unique_lock lock{ m_updated_chunks_mutex };
            if (!m_updated_chunks.empty())
            {
                on_chunks_updated.broadcast({ m_updated_chunks });
                m_updated_chunks.clear();
            }
        }

        {
            std::unique_lock lock{ m_deleted_chunk_columns_mutex };
            if (!m_deleted_chunk_columns.empty())
            {
                on_chunks_deleted.broadcast({ m_deleted_chunk_columns });
                m_deleted_chunk_columns.clear();
            }
        }
    }

    std::shared_ptr<ChunkManager::ChunkColumnData> ChunkManager::create_chunk_column(v2i chunk_column_pos)
    {
        const auto chunk_col = std::make_shared<ChunkColumnData>();
        for (i32 i = 0; i < voxel_constants::vertical_chunk_count; i++)
        {
            (*chunk_col)[i].chunk.init(
                { chunk_column_pos.x, i, chunk_column_pos.y },
                g_engine->get_module_checked<VoxelModule>());
        }

        return chunk_col;
    }

    std::shared_ptr<ChunkManager::ChunkColumnData> ChunkManager::find_chunk_column(v2i chunk_column_pos) const
    {
        const std::shared_lock lock { m_loaded_chunks_mutex };
        if (const auto it = m_loaded_chunks.find(chunk_column_pos); it != m_loaded_chunks.end())
            return it->second;

        return nullptr;
    }

    std::shared_ptr<ChunkManager::ChunkColumnData> ChunkManager::find_or_create_chunk_column(v2i chunk_column_pos)
    {
        if (const auto chunk_column = find_chunk_column(chunk_column_pos))
            return chunk_column;

        const std::unique_lock lock { m_loaded_chunks_mutex };

        const auto [it, success] = m_loaded_chunks.try_emplace(chunk_column_pos, nullptr);
        if (success)
            it->second = create_chunk_column(chunk_column_pos);

        return it->second;
    }
}