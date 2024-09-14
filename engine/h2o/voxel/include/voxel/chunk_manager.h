#pragma once

#include "chunk_view.h"
#include "core/log.h"

#include <glm/gtx/hash.hpp>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <unordered_map>

namespace h2o
{
    struct ChunksUpdatedEvent { const std::unordered_set<v3i>& updated_chunks{}; };
    struct ChunksDeletedEvent { const std::unordered_set<v2i>& deleted_chunk_columns{}; };

    class ChunkManager
    {
    public:

        ChunkManager() = default;
        virtual ~ChunkManager() = default;

        [[nodiscard]] std::optional<Block> get_block_at(const v3i& block_pos) const;
        bool set_block_at(const v3i& block_pos, Block block);

        [[nodiscard]] bool chunk_exists(const v3i& chunk_pos) const;

        void fetch_or_create_chunk_mut(const v3i& chunk_pos, const std::function<void(Chunk*)>& function);
        void fetch_chunk_mut(const v3i& chunk_pos, const std::function<void(Chunk*)>& function);
        void fetch_chunk(const v3i& chunk_pos, const std::function<void(const Chunk*)>& function) const;

        void view_or_create_chunk_column_mut(v2i chunk_column_pos, const std::function<void(ChunkColumnView&)>& function);
        void view_chunk_column_mut(v2i chunk_column_pos, const std::function<void(ChunkColumnView&)>& function);
        void view_chunk_column(v2i chunk_column_pos, const std::function<void(const ChunkColumnView&)>& function) const;

        // TODO: Implement this. Will need to call event.
        // void remove_all_chunk_columns(const std::function<bool(v2i)>& condition);

        // Open a temporary view into a cubic region of chunks, mutable or otherwise
        template<v3i ViewSize>
        void view_or_create_mut(const v3i& corner, const std::function<void(ChunkView<ViewSize>&)>& function);
        template<v3i ViewSize>
        void view_mut(const v3i& corner, const std::function<void(ChunkView<ViewSize>&)>& function);
        template<v3i ViewSize>
        void view(const v3i& corner, const std::function<void(const ChunkView<ViewSize>&)>& function) const;

        void view_for_meshing(const v3i& chunk_pos, const std::function<void(const ChunkMeshingView&)>& function) const;

        void broadcast_events();

        static constexpr bool is_valid_chunk_y(i32 chunk_y)
        { return chunk_y >= 0 && chunk_y < voxel_constants::vertical_chunk_count; }

        Event<ChunksUpdatedEvent> on_chunks_updated{}; // First update is creation
        Event<ChunksDeletedEvent> on_chunks_deleted{};

    private:

        struct ChunkData
        {
            Chunk chunk{};
            std::shared_mutex mutex{};
        };

        using ChunkColumnData = std::array< ChunkData, voxel_constants::vertical_chunk_count >;

        [[nodiscard]] static std::shared_ptr<ChunkColumnData> create_chunk_column(v2i chunk_column_pos);
        [[nodiscard]] std::shared_ptr<ChunkColumnData> find_chunk_column(v2i chunk_column_pos) const;
        [[nodiscard]] std::shared_ptr<ChunkColumnData> find_or_create_chunk_column(v2i chunk_column_pos);

    private:

        std::unordered_map<v2i, std::shared_ptr<ChunkColumnData>> m_loaded_chunks{};
        mutable std::shared_mutex m_loaded_chunks_mutex{};

        // These are stored to ensure events are called on the correct thread.
        // TODO: Make sure nothing bad happens if a chunk is updated and deleted on the same frame
        std::mutex m_updated_chunks_mutex{};
        std::unordered_set<v3i> m_updated_chunks{};
        std::mutex m_deleted_chunk_columns_mutex{};
        std::unordered_set<v2i> m_deleted_chunk_columns{};

    };

    template<v3i ViewSize>
    void ChunkManager::view_or_create_mut(
        const v3i& corner,
        const std::function<void(ChunkView<ViewSize>&)>& function)
    {
        // TODO: Could this be a stack-allocated array ?
        std::vector< std::unique_lock<std::shared_mutex> > chunk_locks{};
        chunk_locks.reserve(ViewSize.x * ViewSize.y * ViewSize.z);

        ChunkView<ViewSize> chunk_view(corner);

        for (i32 i = corner.x; i < corner.x + ViewSize.x; i++)
        for (i32 k = corner.z; k < corner.z + ViewSize.z; k++)
        {
            const auto chunk_col = find_or_create_chunk_column({ i, k });
            assert(chunk_col != nullptr);

            for (i32 j = corner.y; j < corner.y + ViewSize.y; j++)
            {
                if (!is_valid_chunk_y(j))
                    continue;

                auto& [chunk, mutex] = (*chunk_col)[j];
                chunk_view.add_chunk(chunk);
                chunk_locks.emplace_back(mutex);
            }
        }

        function(chunk_view);

        {
            std::unique_lock updated_chunks_lock{ m_updated_chunks_mutex };
            chunk_view.for_each_chunk(
                [&](const Chunk& chunk)
                {
                    m_updated_chunks.insert(chunk.chunk_pos());
                }
            );
        }
    }

    template<v3i ViewSize>
    void ChunkManager::view_mut(
        const v3i& corner,
        const std::function<void(ChunkView<ViewSize>&)>& function)
    {
        std::vector< std::unique_lock<std::shared_mutex> > chunk_locks{};
        chunk_locks.reserve(ViewSize.x * ViewSize.y * ViewSize.z);

        ChunkView<ViewSize> chunk_view(corner);

        for (i32 i = corner.x; i < corner.x + ViewSize.x; i++)
        for (i32 k = corner.z; k < corner.z + ViewSize.z; k++)
        {
            const auto chunk_col = find_chunk_column({ i, k });
            if (!chunk_col)
                continue;

            for (i32 j = corner.y; j < corner.y + ViewSize.y; j++)
            {
                if (!is_valid_chunk_y(j))
                    continue;

                auto& [chunk, mutex] = (*chunk_col)[j];
                chunk_view.add_chunk(chunk);
                chunk_locks.emplace_back(mutex);
            }
        }

        function(chunk_view);

        {
            std::unique_lock updated_chunks_lock{ m_updated_chunks_mutex };
            chunk_view.for_each_chunk(
                [&](const Chunk& chunk)
                {
                    m_updated_chunks.insert(chunk.chunk_pos());
                }
            );
        }
    }

    template<v3i ViewSize>
    void ChunkManager::view(
        const v3i& corner,
        const std::function<void(const ChunkView<ViewSize>&)>& function) const
    {
        std::vector< std::shared_lock<std::shared_mutex> > chunk_locks{};
        chunk_locks.reserve(ViewSize.x * ViewSize.y * ViewSize.z);

        ChunkView<ViewSize> chunk_view(corner);

        for (i32 i = corner.x; i < corner.x + ViewSize.x; i++)
        for (i32 k = corner.z; k < corner.z + ViewSize.z; k++)
        {
            const auto chunk_col = find_chunk_column({ i, k });
            if (!chunk_col)
                continue;

            for (i32 j = corner.y; j < corner.y + ViewSize.y; j++)
            {
                if (!is_valid_chunk_y(j))
                    continue;

                auto& [chunk, mutex] = (*chunk_col)[j];
                chunk_view.add_chunk(chunk);
                chunk_locks.emplace_back(mutex);
            }
        }

        function(chunk_view);
    }
}
