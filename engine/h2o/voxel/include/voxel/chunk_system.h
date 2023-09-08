#pragma once

#include "core/distance_queue.h"
#include "core/events.h"
#include "core/handle_types.h"
#include "scene/scene_system.h"
#include "voxel/server/chunk_server.h" // TEMP
#include "voxel/chunk_region.h"
#include "voxel/voxel_constants.h"

#include <deque>
#include <glm/gtx/hash.hpp>
#include <memory>
#include <queue>
#include <vector>

namespace h2o
{
    class ChunkGenerator_Base;

    class ChunkColumn;
    class ChunkRegionOLD;
    class VoxelModule;
    struct ChunkWeakHandle;

    struct ChunkUpdateEvent { const ChunkWeakHandle& chunk_handle; };
    struct ChunkColumnLoadEvent { const WeakHandle<ChunkColumn>& chunk_handle; };
    struct PlayerChangedChunkEvent { v3i old_chunk_pos; v3i new_chunk_pos; };

    using ChunkFetchCallback = std::function<void(const WeakHandle<ChunkColumn>&)>;

    /**
     * A system meant to be added to scenes that generates a voxel world,
     * allowing for easy access to its chunks.
     */
    class ChunkSystem : public SceneSystem
    {
    public:

        explicit ChunkSystem(
            const SceneSystemInitializer& system_initializer,
            const VoxelModule& voxel_module);

        // SceneSystem interface
        bool init() override;
        void update(f32 delta_time) override;

        [[nodiscard]] i32 num_chunks_waiting_generation() const;

        [[nodiscard]] WeakHandle<ChunkColumn> fetch_chunk_column(v2i chunk_col_pos) const;
        [[nodiscard]] WeakHandle<ChunkColumn> fetch_or_create_chunk_column(v2i chunk_col_pos);

        void request_chunk_generation(
            const WeakHandle<ChunkColumn>& chunk_col,
            i32 queried_stage = voxel_constants::max_generation_stage);
//        void request_chunk_generation(const StaticChunkRegion& chunk_region, i32 queried_stage);

    public:

        Event<ChunkColumnLoadEvent> on_chunk_column_loaded;
        Event<ChunkUpdateEvent> on_chunk_updated;
        Event<PlayerChangedChunkEvent> on_player_changed_chunk;

    protected:

        [[nodiscard]] OwningHandle<ChunkColumn> create_chunk_column(v2i chunk_location) const;

    private:

        struct ChunkGenRequest
        {
            StaticChunkRegion gen_region;
            WeakHandle<ChunkColumn> chunk_column;
            i32 gen_stage;
        };

        std::deque<ChunkGenRequest> m_chunk_gen_stack;
        std::unordered_map< v2i, OwningHandle<ChunkColumn> > m_loaded_chunks;
        std::queue< WeakHandle<ChunkColumn> > m_chunk_tick_queue;

        std::unique_ptr<ChunkGenerator_Base> m_chunk_generator { nullptr };

        v3i m_last_player_chunk_pos{};

        const VoxelModule* const m_voxel_module { nullptr };

        ChunkServer m_chunk_server{};

    };
}