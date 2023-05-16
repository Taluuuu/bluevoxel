#pragma once

#include "core/events.h"
#include "core/handle_types.h"
#include "core/types.h"
#include "scene/scene_system.h"

#include <vector>

namespace h2o
{
    class Chunk;

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

        [[nodiscard]] WeakHandle<Chunk> get_chunk_at(const v3i& world_pos) const;

        Event<ChunkEvent> on_chunk_created;
        Event<ChunkEvent> on_chunk_deleted;
        Event<ChunkEvent> on_chunk_updated;

    private:

        static constexpr size_t to_index(const v3i& world_pos);
        static constexpr bool is_valid_pos(const v3i& world_pos);

    private:

        std::vector< OwningHandle<Chunk> > m_chunks;

        // Constants in chunks
        static constexpr u32 m_world_height = 5;
        static constexpr u32 m_world_size = 5;
        static constexpr u32 m_world_area = m_world_size * m_world_size;
        static constexpr u32 m_world_volume = m_world_area * m_world_height;

    };
}