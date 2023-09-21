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

        std::unordered_map<v2i, std::shared_ptr<ChunkColumn>> m_chunks{};

        Client* m_client = nullptr;

        bool temp = true;

    };
}