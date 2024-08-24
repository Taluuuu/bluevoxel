#pragma once

#include "chunk_view.h"

#include <glm/gtx/hash.hpp>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <unordered_map>

namespace h2o
{
    class ChunkManager
    {
    public:

        ChunkManager() = default;
        virtual ~ChunkManager() = default;

        [[nodiscard]] std::optional<Block> get_block_at(const v3i& block_pos) const;
        bool set_block_at(const v3i& block_pos, Block block, bool replicate);

        void fetch_chunk(const v3i& chunk_pos, const std::function<void(Chunk*)>& function);
        void fetch_chunk(const v3i& chunk_pos, const std::function<void(const Chunk*)>& function) const;

        void view_or_create_chunk_column(v2i chunk_column_pos, const std::function<void(ChunkColumnView&)>& function);
        void view_chunk_column(v2i chunk_column_pos, const std::function<void(ChunkColumnView&)>& function);
        void view_chunk_column(v2i chunk_column_pos, const std::function<void(const ChunkColumnView&)>& function) const;

        // Open a temporary view into a cubic region of chunks, mutable or otherwise
        template<v3u ViewSize>
        void view_or_create(const v3i& corner, const std::function<void(ChunkView<ViewSize>& chunk_view)>& function);
        template<v3u ViewSize>
        void view(const v3i& corner, const std::function<void(ChunkView<ViewSize>& chunk_view)>& function);
        template<v3u ViewSize>
        void view(const v3i& corner, const std::function<void(const ChunkView<ViewSize>& chunk_view)>& function) const;

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

    };

    template <v3u ViewSize>
    void ChunkManager::view_or_create(
        const v3i& corner,
        const std::function<void(ChunkView<ViewSize>& chunk_view)>& function)
    {
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
                if (j >= voxel_constants::vertical_chunk_count)
                    break;

                auto& [chunk, mutex] = (*chunk_col)[j];
                chunk_view.add_chunk(chunk);
                chunk_locks.emplace_back(mutex);
            }
        }

        function(chunk_view);
    }

    template<v3u ViewSize>
    void ChunkManager::view(
        const v3i& corner,
        const std::function<void(ChunkView<ViewSize>& chunk_view)>& function)
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
                if (j >= voxel_constants::vertical_chunk_count)
                    break;

                auto& [chunk, mutex] = (*chunk_col)[j];
                chunk_view.add_chunk(chunk);
                chunk_locks.emplace_back(mutex);
            }
        }

        function(chunk_view);
    }

    template<v3u ViewSize>
    void ChunkManager::view(
        const v3i& corner,
        const std::function<void(const ChunkView<ViewSize>& chunk_view)>& function) const
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
                if (j >= voxel_constants::vertical_chunk_count)
                    break;

                auto& [chunk, mutex] = (*chunk_col)[j];
                chunk_view.add_chunk(chunk);
                chunk_locks.emplace_back(mutex);
            }
        }

        function(chunk_view);
    }
}
