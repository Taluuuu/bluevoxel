#include "voxel/chunk_manager.h"

#include "core/engine.h"
#include "voxel/chunk_view.h"
#include "voxel/voxel_utils.h"

namespace h2o
{
    Block ChunkManager::get_block_at(const v3i& block_pos) const
    {
        Block block = Block::Air;
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
        fetch_chunk_mut(voxel_utils::block_to_chunk_pos(block_pos),
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

    bool ChunkManager::chunk_column_exists(const v2i chunk_column_pos) const
    {
        return find_chunk_column(chunk_column_pos) != nullptr;
    }

    bool ChunkManager::chunk_exists(const v3i& chunk_pos) const
    {
        if (chunk_pos.y >= 0 && chunk_pos.y < voxel_constants::vertical_chunk_count)
            return chunk_column_exists({ chunk_pos.x, chunk_pos.z });

        return false;
    }

    void ChunkManager::fetch_or_create_chunk_mut(const v3i& chunk_pos, const std::function<void(Chunk*)>& function)
    {
        view_or_create_mut<v3i{1}>(chunk_pos,
            [&](ChunkView<v3i{1}>& view)
            {
                function(view.get_chunk_at(chunk_pos));
            }
        );
    }

    void ChunkManager::fetch_chunk_mut(const v3i& chunk_pos, const std::function<void(Chunk*)>& function)
    {
        view_mut<v3i{1}>(chunk_pos,
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

    void ChunkManager::view_or_create_chunk_column_mut(
        v2i chunk_column_pos,
        const std::function<void(ChunkColumnView&)>& function)
    {
        view_or_create_mut({ chunk_column_pos.x, 0, chunk_column_pos.y }, function);
    }

    void ChunkManager::view_chunk_column_mut(
        v2i chunk_column_pos,
        const std::function<void(ChunkColumnView&)>& function)
    {
        view_mut({ chunk_column_pos.x, 0, chunk_column_pos.y }, function);
    }

    void ChunkManager::view_chunk_column(
        v2i chunk_column_pos,
        const std::function<void(const ChunkColumnView&)>& function) const
    {
        view({ chunk_column_pos.x, 0, chunk_column_pos.y }, function);
    }

    void ChunkManager::remove_all_chunk_columns(const std::function<bool(v2i)>& condition)
    {
        std::vector<v2i> chunk_columns_to_delete{};

        {
            const std::shared_lock loaded_chunks_lock{ m_loaded_chunks_mutex };
            for (const auto& [chunk_col_pos, _] : m_loaded_chunks)
            {
                if (condition(chunk_col_pos))
                    chunk_columns_to_delete.push_back(chunk_col_pos);
            }
        }

        if (chunk_columns_to_delete.empty())
            return;

        add_to_deleted_chunks_list(chunk_columns_to_delete);

        {
            const std::unique_lock loaded_chunks_lock{ m_loaded_chunks_mutex };
            for (const v2i chunk_col_pos : chunk_columns_to_delete)
                m_loaded_chunks.erase(chunk_col_pos);
        }
    }

    void ChunkManager::view_for_meshing(
        const v3i& chunk_pos,
        const std::function<void(const ChunkMeshingView&)>& function) const
    {
        std::vector< std::shared_lock<std::shared_mutex> > chunk_locks{};
        chunk_locks.reserve(7);

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
                    chunk_view.add_chunk(chunk, neighbour_col);
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
            const std::unique_lock lock{ m_deleted_chunk_columns_mutex };
            if (!m_deleted_chunk_columns.empty())
            {
                on_chunks_deleted.broadcast({ m_deleted_chunk_columns });
                m_deleted_chunk_columns.clear();
            }
        }

        {
            const std::unique_lock lock{ m_updated_chunks_mutex };
            if (!m_updated_chunks.empty())
            {
                on_chunks_updated.broadcast({ m_updated_chunks });
                m_updated_chunks.clear();
            }
        }
    }

    std::shared_ptr<ChunkColumnData> ChunkManager::create_chunk_column(v2i chunk_column_pos)
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

    std::shared_ptr<ChunkColumnData> ChunkManager::find_chunk_column(v2i chunk_column_pos) const
    {
        const std::shared_lock lock { m_loaded_chunks_mutex };
        if (const auto it = m_loaded_chunks.find(chunk_column_pos); it != m_loaded_chunks.end())
            return it->second;

        return nullptr;
    }

    std::shared_ptr<ChunkColumnData> ChunkManager::find_or_create_chunk_column(v2i chunk_column_pos)
    {
        if (const auto chunk_column = find_chunk_column(chunk_column_pos))
            return chunk_column;

        const std::unique_lock lock { m_loaded_chunks_mutex };

        const auto [it, success] = m_loaded_chunks.try_emplace(chunk_column_pos, nullptr);
        if (success)
            it->second = create_chunk_column(chunk_column_pos);

        return it->second;
    }

    void ChunkManager::add_to_updated_chunks_list(const std::vector<v3i>& updated_chunks)
    {
        {
            const std::unique_lock lock{ m_updated_chunks_mutex };
            for (const v3i chunk_col_pos : updated_chunks)
                m_updated_chunks.insert(chunk_col_pos);
        }

        {
            const std::unique_lock lock{ m_deleted_chunk_columns_mutex };
            for (const v3i& chunk_pos : updated_chunks)
            {
                erase_if(m_deleted_chunk_columns,
                    [&](v2i chunk_col_pos)
                    {
                        return chunk_pos.x == chunk_col_pos.x && chunk_pos.z == chunk_col_pos.y;
                    }
                );
            }
        }
    }

    void ChunkManager::add_to_deleted_chunks_list(const std::vector<v2i>& deleted_chunks)
    {
        {
            const std::unique_lock lock{ m_deleted_chunk_columns_mutex };
            for (const v2i chunk_col_pos : deleted_chunks)
                m_deleted_chunk_columns.insert(chunk_col_pos);
        }

        {
            const std::unique_lock lock{ m_updated_chunks_mutex };
            for (const v2i chunk_col_pos : deleted_chunks)
            {
                erase_if(m_updated_chunks,
                    [&](const v3i& chunk_pos)
                    {
                        return chunk_pos.x == chunk_col_pos.x && chunk_pos.z == chunk_col_pos.y;
                    }
                );
            }
        }
    }
}
