#pragma once

#include "core/events.h"
#include "core/handle_types.h"
#include "core/types.h"
#include "scene/scene_system.h"

#include <glm/gtx/hash.hpp>
#include <memory>
#include <vector>

namespace h2o
{
    class Chunk;
    class ChunkGenerator_Base;

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
        void update(f32 delta_time) override;

        Event<ChunkEvent> on_chunk_created;
        Event<ChunkEvent> on_chunk_deleted;
        Event<ChunkEvent> on_chunk_updated;

    private:

        using ChunkPtr = std::shared_ptr<Chunk>;
        std::vector<ChunkPtr> m_loaded_chunks;
        std::unordered_map<v2i, std::vector<ChunkPtr>> m_world_chunks;

        v3i m_last_player_chunk_pos{};

        std::unique_ptr<ChunkGenerator_Base> m_chunk_generator { nullptr };

        // Constants in chunks
        static constexpr u32 m_world_height = 5;
        static constexpr u32 m_world_size = 5;
        static constexpr u32 m_world_area = m_world_size * m_world_size;
        static constexpr u32 m_world_volume = m_world_area * m_world_height;

    };
}