#include "scene/scene.h"

#include "core/engine.h"
#include "scene/scene_module.h"

namespace h2o
{
    Scene::~Scene()
    {
        // TODO: Make sure m_engine is still alive at this point
        if (auto scene_module = m_engine->get_module<SceneModule>())
            scene_module->register_scene(*this);
    }

    Scene::Scene(Engine& engine, std::string_view name)
        : m_engine(&engine)
    {

    }

    std::shared_ptr<Scene> Scene::create(Engine& engine, std::string_view name)
    {
        auto* scene = new Scene(engine, name);

        if (auto scene_module = engine.get_module<SceneModule>())
            scene_module->register_scene(*scene);

        return std::shared_ptr<Scene>(scene);
    }

    void Scene::tick(f32 delta_time)
    {
        for (const auto& actor : m_actor_map)
        {
            assert(actor.second);
            actor.second->tick(delta_time);
        }
    }

    void Scene::set_main_camera(const OwningHandle<gfx::Camera>& camera)
    {
        m_main_camera = camera;
        log::info("Set new main camera");
    }
}