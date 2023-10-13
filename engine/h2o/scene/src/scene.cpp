#include "scene/scene.h"

#include "core/engine.h"
#include "scene/scene_module.h"

namespace h2o
{
    Scene::~Scene()
    {
        m_actor_map.clear();
        m_system_map.clear();

        if (auto scene_module = m_engine->get_module<SceneModule>())
            scene_module->unregister_scene(*this);
    }

    std::shared_ptr<Scene> Scene::create(Engine& engine, std::string_view name)
    {
        auto* scene = new Scene(engine, name);

        if (auto scene_module = engine.get_module<SceneModule>())
            scene_module->register_scene(*scene);

        return std::shared_ptr<Scene>(scene);
    }

    bool Scene::init()
    {
        bool success = true;
        for (const auto& system : m_system_map)
        {
            if (!system.second || !system.second->init())
                success = false;
        }

        if (!success)
            log::warn("Failed to initialize all systems for scene: {}", m_scene_name);

        return success;
    }

    void Scene::tag_actor(const WeakHandle<Actor>& actor, ActorTag tag)
    {
        const ActorTag previous_tag = actor->get_tag();
        if (tag == previous_tag)
            return;

        m_actor_tags[previous_tag] = nullptr;
        if (tag != ActorTag::None)
            m_actor_tags[tag] = actor;

        actor->m_actor_tag = tag;
    }

    Scene::Scene(Engine& engine, std::string_view name)
        : m_engine(&engine)
        , m_scene_name(name)
    {
        if (auto scene_module = m_engine->get_module<SceneModule>())
            scene_module->register_scene(*this);
    }
}