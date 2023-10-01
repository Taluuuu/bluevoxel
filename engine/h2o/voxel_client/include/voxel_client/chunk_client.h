#pragma once

#include "networking/client.h"
#include "scene/scene_system.h"
#include "voxel_rendering/chunk_mesh.h"

#include <glm/gtx/hash.hpp>
#include <memory>
#include <unordered_map>

namespace h2o
{
    class ChunkColumn;

    class ChunkClient : public SceneSystem
    {
    public:

        explicit ChunkClient(
            const SceneSystemInitializer& system_initializer,
            Client& client);
        ~ChunkClient() override = default;

        // Tickable interface
        void update(f32 delta_time) override;

    private:

        void request_chunk_loads();
        void trim_far_chunks();

    private:

        struct ChunkData
        {
            std::shared_ptr<ChunkColumn> chunk_column = nullptr;
            size_t chunk_mesh_index = 0;
        };

        struct ChunkMeshData
        {
            ChunkMesh chunk_mesh{};
            bool can_be_remeshed = true;
        };

        // If there is an entry in the map, the chunk has been requested.
        std::unordered_map<v2i, ChunkData> m_chunks{};
        std::vector<ChunkMeshData> m_chunk_mesh_pool{};

        Client* m_client = nullptr;
        bool m_refresh_chunk_requests = false;

        EventHandle m_on_connected_handle{};
        EventHandle m_on_fetched_chunk_handle{};

        i32 m_view_distance = 5;
        i32 m_stay_loaded_distance = 3;

        v2i m_previous_player_chunk_col_pos{};

    };
}