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

        // Request chunk generation, does not happen instantly
        void generate_regions_for_chunk(v2i chunk_pos);
        [[nodiscard]] std::vector<v2i> pop_newly_generated_regions();

        // Fetch the chunk region containing the input chunk pos. Null if does not exist.
        void fetch_region(v2i region_pos, const std::function<void(const ChunkRegion*)>& function) const;
        void fetch_region(v2i region_pos, const std::function<void(ChunkRegion*)>& function);

    private:

        struct ChunkRegionData
        {
            explicit ChunkRegionData(v2i region_pos = {})
                : chunk_region(region_pos) {}

            ChunkRegion chunk_region;
            mutable std::shared_mutex mutex{};
        };

        void fetch_region_data(v2i region_pos, const std::function<void(const ChunkRegionData*)>& function) const;
        void fetch_region_data(v2i region_pos, const std::function<void(ChunkRegionData*)>& function);

        [[nodiscard]] std::vector<v2i> get_regions_to_generate_for_chunk(v2i chunk_pos) const;

        // For a region to be fully generated, all its neighbors must be generated.
        [[nodiscard]] bool is_region_fully_generated(v2i region_pos) const;
        // Mark generated all chunks in regions for which all neighboring regions are generated.
        void update_chunk_generation_states(v2i region_pos) const;

        void generate_region(v2i region_pos);

    private:

        std::unordered_map<v2i, std::shared_ptr<ChunkRegionData>> m_chunk_regions{};
        mutable std::shared_mutex m_chunk_regions_mutex{};

        std::vector<v2i> m_newly_generated_regions{};
        mutable std::mutex m_newly_generated_regions_mutex{};

        ChunkManager* const m_chunk_manager = nullptr;
        std::shared_ptr<ChunkGenerator_Base> m_chunk_generator = nullptr;

    };
}
