#pragma

#include "core/log.h"
#include "actor.h"

#include <unordered_map>
#include <vector>

namespace h2o
{
    class Actor;

    class Scene : public std::enable_shared_from_this<Scene>
    {
    public:

        Scene() = delete;
        Scene(const Scene&) = delete;
        Scene(Scene&&) = delete;

    private:

        explicit Scene(std::string_view name);

    public:

        static std::shared_ptr<Scene> create(std::string_view name);

        /**
         * Create and store a new actor of type T
         *
         * @tparam T
         * @tparam Args
         * @param name
         * @param args
         * @return
         */
        template<class T = Actor, typename... Args>
        std::shared_ptr<Actor> create_actor(std::string_view name, Args... args);

        template<class T = Actor>
        std::shared_ptr<T> get_actor(std::string_view name);

    private:

        std::unordered_map< std::string_view, std::shared_ptr<Actor> > m_actor_map;

    };

    template<class T, typename... Args>
    std::shared_ptr<Actor> Scene::create_actor(std::string_view name, Args... args)
    {
        static_assert(std::is_base_of_v<Actor, T>, "T must derive from h2o::Actor.");

        if (get_actor(name) != nullptr)
        {
            log::error("Tried to create multiple actors with the same name '{}'", name);
            return nullptr;
        }

        ActorInitializer actor_initializer
        {
            .actor_name = name,
            .scene = shared_from_this()
        };

        std::shared_ptr<Actor> actor = std::make_shared<T>(actor_initializer, args...);
    }

    template<class T>
    std::shared_ptr<T> Scene::get_actor(std::string_view name)
    {
        static_assert(std::is_base_of_v<Actor, T>, "T must derive from h2o::Actor.");

        auto actor_it = m_actor_map.find(name);
        if (actor_it == m_actor_map.end())
            return nullptr;

        return std::dynamic_pointer_cast<T>(actor_it->second);
    }
}