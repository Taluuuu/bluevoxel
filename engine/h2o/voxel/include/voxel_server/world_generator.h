#pragma once

#include "core/events.h"
#include "core/types.h"
#include "voxel/chunk_generators/chunk_generator_base.h"

#include <deque>
#include <functional>
#include <glm/gtx/hash.hpp>
#include <memory>
#include <mutex>
#include <vector>

namespace h2o
{
    // class ChunkColumn;
    // class ChunkManager_Base;
    //
    // using ChunkRequestFinishedCallback = std::function<void(const ChunkColumn&)>;
    //
    // class WorldGenerator
    // {
    // public:
    //
    //     explicit WorldGenerator(ChunkManager_Base& chunk_mgr);
    //
    //     void request_chunk_column(
    //         v2i chunk_column_pos,
    //         const ChunkRequestFinishedCallback& on_completed);
    //
    //     void set_chunk_generator(std::unique_ptr<ChunkGenerator_Base>&& chunk_generator);
    //
    //     [[nodiscard]] size_t num_pending_chunks() const { return m_chunks_pending_generation.size(); }
    //
    // private:
    //
    //     void generate_chunk(ChunkColumn& chunk_column);
    //
    // private:
    //
    //     std::unordered_map<v2i, std::vector<ChunkRequestFinishedCallback>> m_chunks_pending_generation{};
    //     std::mutex m_mutex{};
    //
    //     std::unique_ptr<const ChunkGenerator_Base> m_chunk_generator = nullptr;
    //
    //     ChunkManager_Base* const m_chunk_mgr = nullptr;
    //
    // };
}