#pragma

#include "actor.h"
#include "core/events.h"
#include "core/log.h"
#include "core/tickable.h"
#include "scene_system.h"

#include <unordered_map>
#include <vector>

namespace h2o
{
    class Engine;
    class INetPeer;
    class SceneSystem;

    class Scene : public Tickable
    {
    public:

        Scene(const std::string& scene_name, INetPeer* net_peer);
        Scene(const Scene&) = delete;

        // A scene currently registers itself to the SceneModule by its memory address,
        // so we want to make sure it doesn't move
        Scene(Scene&&) = delete;
        ~Scene() override;

        bool init();
        void cleanup();

        /**
         * Create and store a new actor of type T
         *
         * @tparam T The actor's type, must inherit from or be Actor
         * @param spawn_transform The transform to apply to the actor on spawn
         * @param actor_id_override An optional override for the actor's id
         * @return The created actor or nullptr on failure
         */
        template<class T = Actor>
        WeakHandle<T> spawn_actor(const Transform& spawn_transform = Transform{}, ActorID actor_id_override = 0);

        bool destroy_actor(ActorID actor_id, bool replicate = true);

        /**
         * Get the actor of type T with a name
         *
         * @tparam T The actor's type
         * @param name The actor's unique name
         * @return The found actor or nullptr on failure
         */
        template<class T = Actor>
        WeakHandle<T> get_actor(ActorID actor_id);

        template<class T = Actor>
        WeakHandle<T> get_actor_by_tag(ActorTag actor_tag);

        template<class T, class... Args>
        WeakHandle<T> add_system(Args... args);

        template<class T>
        WeakHandle<T> get_system();

        void tag_actor(const WeakHandle<Actor>& actor, ActorTag tag);

    private:

        std::unordered_map< ActorID, OwningHandle<Actor> > m_actor_map{};
        std::unordered_map< ActorTag, WeakHandle<Actor> > m_actor_tags{};
        ActorID m_actor_id_generator = 1;

        std::unordered_map< std::type_index, OwningHandle<SceneSystem> > m_system_map{};

        std::string m_scene_name{};

        INetPeer* m_net_peer = nullptr;
        EventHandle m_on_object_destroyed_handle{};

    };

    template<class T>
    WeakHandle<T> Scene::spawn_actor(const Transform& spawn_transform, ActorID actor_id_override)
    {
        static_assert(
            std::is_base_of_v<Actor, T>, "T must derive from h2o::Actor.");

        ActorID actor_id = actor_id_override;
        if (actor_id == 0)
        {
            actor_id = m_actor_id_generator++;
        }
        else
        {
            // fuck fuck fuck fuck
            if (get_actor(actor_id_override) != nullptr)
            {
                assert(false);
                return nullptr;
            }
        }

        const ActorInitializer actor_initializer { actor_id, *this };

        OwningHandle<T> actor = oup::make_observable_unique<T>(actor_initializer);
        WeakHandle<T> weak_actor = actor;
        actor->transform = spawn_transform;

        auto [it, success] = m_actor_map.insert({ actor_id, std::move(actor) });
        assert(success);

        return weak_actor;
    }

    template<class T>
    WeakHandle<T> Scene::get_actor(ActorID actor_id)
    {
        static_assert(
            std::is_base_of_v<Actor, T>, "T must derive from h2o::Actor.");

        auto actor_it = m_actor_map.find(actor_id);
        if (actor_it == m_actor_map.end())
            return nullptr;

        WeakHandle<Actor> actor = actor_it->second;
        return oup::dynamic_pointer_cast<T>(actor);
    }

    template<class T>
    WeakHandle<T> Scene::get_actor_by_tag(ActorTag actor_tag)
    {
        static_assert(
            std::is_base_of_v<Actor, T>, "T must derive from h2o::Actor.");

        auto actor_it = m_actor_tags.find(actor_tag);
        if (actor_it == m_actor_tags.end())
            return nullptr;

        WeakHandle<Actor> actor = actor_it->second;
        return oup::dynamic_pointer_cast<T>(actor);
    }

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

        SceneSystemInitializer system_initializer
        {
            .owning_scene = *this
        };

        OwningHandle<T> system = oup::make_observable_unique<T>(system_initializer, args...);
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