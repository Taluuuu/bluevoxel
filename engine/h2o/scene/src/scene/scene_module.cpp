#include "scene/scene_module.h"

#include "networking/networking_utils.h"
#include "scene/scene.h"
#include "scene/scene_networking_components.h"

#include <entt/core/hashed_string.hpp>
#include <entt/meta/factory.hpp>
#include <entt/meta/context.hpp>
#include <entt/meta/factory.hpp>
#include <entt/meta/meta.hpp>
#include <bitsery/bitsery.h>
#include <bitsery/adapter/buffer.h>
#include <bitsery/traits/vector.h>
#include <bitsery/traits/string.h>
#include <bitsery/brief_syntax.h>

using namespace entt::literals;

namespace h2o
{
    bool SceneModule::init(Engine& engine)
    {
        register_component<NetworkSync>();

        entt::registry test_reg;
        net_utils::Buffer buffer;

        {
            const auto test_ent = test_reg.create();
            test_reg.emplace<NetworkSync>(test_ent, 6);

            log::info("{}", test_reg.get<NetworkSync>(test_ent).owner);

            net_utils::Writer writer{ buffer };
            serialize_entity(test_reg, test_ent, writer);
        }

        {
            net_utils::Reader reader{ buffer.begin(), buffer.size() };
            const auto test_ent = deserialize_entity(test_reg, reader);

            log::info("{}", test_reg.get<NetworkSync>(test_ent).owner);
        }

        return true;
    }

    void SceneModule::on_engine_starts_closing()
    {
        for (auto scene : m_scenes)
            scene->cleanup();
    }

    void SceneModule::register_scene(Scene& scene)
    {
        m_scenes.insert(&scene);
    }

    void SceneModule::unregister_scene(Scene& scene)
    {
        m_scenes.erase(&scene);
    }

    entt::entity SceneModule::deserialize_entity(
        entt::registry& registry,
        net_utils::Reader& reader) const
    {
        const auto entity = registry.create();

        u64 num_components{};
        reader.value8b(num_components);

        for (u64 i = 0; i < num_components; ++i)
        {
            entt::id_type type{};
            reader.value4b(type);

            if (const auto it = m_registered_components.find(type); it != m_registered_components.end())
                it->second.deserialize(registry, entity, reader);
        }

        return entity;
    }

    void SceneModule::serialize_entity(
        const entt::registry& registry,
        const entt::entity entity,
        net_utils::Writer& writer) const
    {
        std::vector<entt::id_type> component_types{};
        for (const auto& type : m_registered_components | std::views::keys)
        {
            if (const auto storage = registry.storage(type))
            {
                if (storage->contains(entity))
                    component_types.emplace_back(type);
            }
        }

        writer.value8b(component_types.size());
        for (const auto& type : component_types)
        {
            writer.value4b(type);
            if (const auto it = m_registered_components.find(type); it != m_registered_components.end())
                it->second.serialize(registry, entity, writer);
        }
    }
}
