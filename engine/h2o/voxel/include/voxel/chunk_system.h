#pragma once

#include "chunk_ptr.h"
#include "core/events.h"
#include "core/handle_types.h"
#include "scene/scene_system.h"

#include <glm/gtx/hash.hpp>
#include <memory>
#include <vector>

namespace h2o
{
    class ChunkGenerator_Base;

    struct ChunkEvent { Chunk& chunk; };
    struct PlayerChangedChunkEvent { v3i old_chunk_pos; v3i new_chunk_pos; };

//    struct ChunkFetchResult
//    {
//
//    };

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

        [[nodiscard]] ChunkColumnPtr fetch_chunk_column(v2i chunk_location) const;
        void fetch_or_create_chunk_column(
            v2i chunk_location,
            const std::function<void(const ChunkColumnPtr&)>& chunk_fetch_callback);

        Event<ChunkEvent> on_chunk_loaded;
        Event<ChunkEvent> on_chunk_unloaded;
        Event<ChunkEvent> on_chunk_updated;
        Event<PlayerChangedChunkEvent> on_player_changed_chunk;

    protected:

        ChunkColumnPtr create_chunk_column(v2i chunk_location) const;

    private:

        std::unordered_map<v2i, ChunkColumnPtr> m_loaded_chunks;

        std::unique_ptr<ChunkGenerator_Base> m_chunk_generator { nullptr };

        v3i m_last_player_chunk_pos{};

    };
}