#pragma

#include "core/tickable.h"
#include "core/log.h"
#include "actor.h"
#include "scene_system.h"

#include <unordered_map>
#include <vector>

namespace h2o
{
    class Engine;
    class SceneSystem;

    class Scene final : public std::enable_shared_from_this<Scene>
    {
    public:

        Scene() = delete;
        Scene(const Scene&) = delete;

        // A scene currently registers itself to the SceneModule by its memory address,
        // so we want to make sure it doesn't move
        Scene(Scene&&) = delete;
        virtual ~Scene();

        /**
         * Create a new scene
         *
         * @param name The scene's name
         * @return The created scene or nullptr on failure
         */
        static std::shared_ptr<Scene> create(Engine& engine, std::string_view name);

        bool init();

        /**
         * Create and store a new actor of type T
         *
         * @tparam T The actor's type, must inherit from or be Actor
         * @tparam Args The actor's constructor argument types
         * @param name The name of the actor, must be unique
         * @param args The actor's constructor arguments
         * @return The created actor or nullptr on failure
         */
        template<class T = Actor, typename... Args>
        WeakHandle<T> spawn_actor(std::string_view name, Args... args);

        /**
         * Get the actor of type T with a name
         *
         * @tparam T The actor's type
         * @param name The actor's unique name
         * @return The found actor or nullptr on failure
         */
        template<class T = Actor>
        WeakHandle<T> get_actor(std::string_view name);

        template<class T, class... Args>
        WeakHandle<T> add_system(Args... args);

        template<class T>
        WeakHandle<T> get_system();

    private:

        explicit Scene(Engine& engine, std::string_view name);

    private:

        std::unordered_map< std::string_view, OwningHandle<Actor> > m_actor_map;

        std::unordered_map< std::type_index, OwningHandle<SceneSystem> > m_system_map;

        std::string_view m_name;

        Engine* const m_engine = nullptr;

    };

    template<class T, typename... Args>
    WeakHandle<T> Scene::spawn_actor(std::string_view name, Args... args)
    {
        static_assert(
            std::is_base_of_v<Actor, T>, "T must derive from h2o::Actor.");

        if (get_actor(name) != nullptr)
        {
            log::error("Tried to create multiple actors with the same name '{}'", name);
            return nullptr;
        }

        ActorInitializer actor_initializer
        {
            .actor_name = name,
            .scene = *this
        };

        //auto test = m_actor_map.try_emplace(name, std::forward(oup::make_observable_unique<T>(actor_initializer, args...)));
        // Returns something useful

        OwningHandle<T> actor = oup::make_observable_unique<T>(actor_initializer, args...);
        WeakHandle<T> weak_actor_handle = actor;

        m_actor_map.insert({ name, std::move(actor) });

        return weak_actor_handle;
    }

    template<class T>
    WeakHandle<T> Scene::get_actor(std::string_view name)
    {
        static_assert(
            std::is_base_of_v<Actor, T>, "T must derive from h2o::Actor.");

        auto actor_it = m_actor_map.find(name);
        if (actor_it == m_actor_map.end())
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

        auto system_it = m_system_map.find(typeid(T));
        if (system_it == m_system_map.end())
            return nullptr;

        WeakHandle<SceneSystem> system = system_it->second;
        return oup::dynamic_pointer_cast<T>(system);
    }
}