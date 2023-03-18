#include "scene/scene.h"

namespace h2o
{
    Scene::Scene(std::string_view name)
    {

    }

    std::shared_ptr<Scene> Scene::create(std::string_view name)
    {
        return std::shared_ptr<Scene>(new Scene(name));
    }
}