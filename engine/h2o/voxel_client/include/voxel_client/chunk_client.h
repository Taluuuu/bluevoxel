#pragma once

#include "networking/client.h"
#include "scene/scene_system.h"

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

        // If there is an entry in the map, the chunk has been requested.
        std::unordered_map<v2i, std::shared_ptr<ChunkColumn>> m_chunks{};

        Client* m_client = nullptr;
        EventHandle m_on_connected_handle{};
        bool m_refresh_chunk_requests = false;

        i32 m_view_distance = 5;
        i32 m_stay_loaded_distance = 3;

        v2i m_previous_player_chunk_col_pos{};

    };
}