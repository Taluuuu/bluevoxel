#pragma once

#include "chunk_region.h"
#include "core/types.h"

#include <functional>
#include <glm/gtx/hash.hpp>
#include <memory>
#include <shared_mutex>
#include <unordered_map>
#include <unordered_set>

namespace h2o
{
    class ChunkGenerator_Base;
    class ChunkServer;
    class ChunkRegion;

    class ChunkRegionManager
    {
    public:

        explicit ChunkRegionManager(ChunkServer& chunk_server);

        // Request chunk generation, does not happen instantly
        void generate_regions_for_chunk(v2i chunk_pos);

        // Fetch the chunk region containing the input chunk pos. Null if does not exist.
        void fetch_region(v2i region_pos, const std::function<void(const ChunkRegion*)>& function) const;
        void fetch_region_mut(v2i region_pos, const std::function<void(ChunkRegion*)>& function);

    private:

        struct ChunkRegionData
        {
            explicit ChunkRegionData(v2i region_pos = {})
                : chunk_region(region_pos) {}

            ChunkRegion chunk_region;
            mutable std::shared_mutex mutex{};
        };

        void fetch_region_data(v2i region_pos, const std::function<void(const ChunkRegionData*)>& function) const;
        void fetch_region_data_mut(v2i region_pos, const std::function<void(ChunkRegionData*)>& function);

        void generate_region_terrain(v2i region_pos);
        void update_chunk_generation_states(v2i region_pos);

    private:

        std::unordered_map<v2i, std::shared_ptr<ChunkRegionData>> m_chunk_regions{};
        mutable std::shared_mutex m_chunk_regions_mutex{};

        ChunkServer* const m_chunk_server = nullptr;

    };
}
