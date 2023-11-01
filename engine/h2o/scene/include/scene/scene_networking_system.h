#pragma once

#include "core/events.h"
#include "core/handle_types.h"
#include "scene_system.h"

#include <memory>
#include <vector>

namespace h2o
{
    class Actor;
    class Client;

    class SceneNetworkingSystem : public SceneSystem
    {
    public:

        explicit SceneNetworkingSystem(
            const SceneSystemInitializer& system_initializer,
            Client& client);
        ~SceneNetworkingSystem() override = default;

        // Tickable interface
        void network_update(f32 delta_time) override;

        void replicate_actor_transform(const WeakHandle<Actor>& actor);
        void stop_replicating_actor_transform(const WeakHandle<Actor>& actor);


    private:

        std::vector< WeakHandle<Actor> > m_replicated_actors{};

        Client* m_client = nullptr;

        EventHandle m_on_received_transform_update_handle{};

    };
}