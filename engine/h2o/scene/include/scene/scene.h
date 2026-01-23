#pragma

#include "actor.h"
#include "core/events.h"
#include "core/log.h"
#include "core/tickable.h"
#include "scene_system.h"

#include <entt/entt.hpp>
#include <unordered_map>
#include <vector>

namespace h2o
{
    class SceneModule;
    class Engine;
    class INetPeer;
    class SceneSystem;

    class Scene : public Tickable
    {
    public:

        Scene(std::string  scene_name, INetPeer* net_peer);
        Scene(const Scene&) = delete;

        // A scene currently registers itself to the SceneModule by its memory address,
        // so we want to make sure it doesn't move
        Scene(Scene&&) = delete;
        ~Scene() override;

        template<class T, class... Args>
        WeakHandle<T> add_system(Args... args);

        template<class T>
        WeakHandle<T> get_system();

        [[nodiscard]] INetPeer* net_peer() const { return m_net_peer; }
        [[nodiscard]] u32 local_peer_id() const;
        [[nodiscard]] entt::registry& registry() { return m_registry; }

        // Called on next network sync so there is time for all components to be added
        Event<entt::entity> on_network_sync_entity_created{};

    protected:

        [[nodiscard]] SceneSystemInitializer make_system_initializer();

        void frame_start() override;
        void network_update(f32 delta_time) override;

    private:

        void on_network_sync_created(entt::entity entity);

    private:

        entt::registry m_registry;
        // Entities that have been created since last network update that have the NetworkSync component
        std::vector<entt::entity> m_newly_spawned_entities{};

        std::unordered_map<u32, entt::entity> m_server_to_local{};
        u32 m_entity_id_generator = 18;

        std::unordered_map< std::type_index, OwningHandle<SceneSystem> > m_system_map{};

        std::string m_scene_name{};

        std::vector<WeakHandle<Actor>> m_actors_to_run_start{};

        INetPeer* m_net_peer = nullptr;
        EventHandle m_on_object_destroyed_handle{};
        EventHandle m_on_spawn_entity_handle{};
        EventHandle m_on_update_component_handle{};
        EventHandle m_on_player_joined_handle{};

        SceneModule& m_scene_module;

    };

    template<class T, class... Args>
    WeakHandle<T> Scene::add_system(Args... args)
    {
        static_assert(
            std::is_base_of_v<SceneSystem, T> &&
            !std::is_same_v<SceneSystem, T>, "T must derive from h2o::SceneSystem.");

        if (get_system<T>() != nullptr)
        {
            log::error("Tried to add multiple scene systems of the same type.");
            return nullptr;
        }

        OwningHandle<T> system = oup::make_observable_unique<T>(make_system_initializer(), args...);
        WeakHandle<T> weak_system_handle = system;

        m_system_map.insert({ typeid(T), std::move(system) });

        return weak_system_handle;
    }

    template<class T>
    WeakHandle<T> Scene::get_system()
    {
        static_assert(
            std::is_base_of_v<SceneSystem, T> &&
            !std::is_same_v<SceneSystem, T>, "T must derive from h2o::SceneSystem.");

        const auto system_it = m_system_map.find(typeid(T));
        if (system_it == m_system_map.end())
            return nullptr;

        const WeakHandle<SceneSystem> system = system_it->second;
        return oup::dynamic_pointer_cast<T>(system);
    }
}
