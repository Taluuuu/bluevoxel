#pragma once

#include "core/events.h"
#include "core/types.h"
#include "scene/scene_system.h"

#include <vector>
#include <memory>

namespace h2o
{
    class Chunk;
    using ChunkPtr = std::unique_ptr<Chunk>;

    struct ChunkEvent { Chunk& chunk; };

    /**
     * A system meant to be added to scenes that generates a voxel world,
     * allowing for easy access to its chunks.
     */
    class ChunkSystem : public SceneSystem
    {
    public:

        explicit ChunkSystem(const SceneSystemInitializer& system_initializer);

        // SceneSystem interface
        bool init() override;

        [[nodiscard]] Chunk* get_chunk_at(const v3i& world_pos) const;
        [[nodiscard]] const std::vector<ChunkPtr>& get_chunks() const;

        // Called when a chunk is modified
        Event<ChunkEvent> on_chunk_updated;

        // Called when a chunk is loaded
        Event<ChunkEvent> on_chunk_loaded;

    private:

        static constexpr size_t to_index(const v3i& world_pos);
        static constexpr bool is_valid_pos(const v3i& world_pos);

    private:

        std::vector<ChunkPtr> m_chunks;

        // Constants in chunks
        static constexpr u32 m_world_height = 1;
        static constexpr u32 m_world_size = 5;
        static constexpr u32 m_world_area = m_world_size * m_world_size;
        static constexpr u32 m_world_volume = m_world_area * m_world_height;

    };
}