#pragma once

#include <memory>
#include <string_view>
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

        Scene(std::string_view name);

    public:

        static std::shared_ptr<Scene> create(std::string_view name);

        std::shared_ptr<Actor> create_actor(std::string_view name);


    private:

        std::vector< std::shared_ptr<Actor> > m_actors;

    };
}