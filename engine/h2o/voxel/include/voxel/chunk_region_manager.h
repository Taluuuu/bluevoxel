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
    class ChunkManager;
    class ChunkRegion;

    class ChunkRegionManager
    {
    public:

        explicit ChunkRegionManager(
            ChunkManager& chunk_manager,
            const std::shared_ptr<ChunkGenerator_Base>& chunk_generator);

        void generate_regions_for_chunk(v2i chunk_pos);

        // Fetch the chunk region containing the input chunk pos. Null if does not exist.
        void fetch_region(v2i region_pos, const std::function<void(const ChunkRegion* region)>& function) const;
        void fetch_region(v2i region_pos, const std::function<void(ChunkRegion* region)>& function);

    private:

        [[nodiscard]] std::vector<v2i> get_regions_to_generate_before_chunk(v2i chunk_pos) const;

        void generate_region(v2i region_pos);
        void on_finished_generating_region();

    private:

        struct ChunkRegionData
        {
            explicit ChunkRegionData(v2i region_pos = {})
                : chunk_region(region_pos) {}

            ChunkRegion chunk_region;
            mutable std::shared_mutex mutex{};
        };

        std::unordered_map<v2i, std::shared_ptr<ChunkRegionData>> m_chunk_regions{};
        std::unordered_set<v2i> m_regions_pending_generation{};
        mutable std::shared_mutex m_chunk_regions_mutex{};

        ChunkManager* const m_chunk_manager = nullptr;
        std::shared_ptr<ChunkGenerator_Base> m_chunk_generator = nullptr;

    };
}
