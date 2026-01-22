#include "scene/scene_module.h"

#include "networking/networking_utils.h"
#include "scene/player.h"
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
        register_component<Position>();
        register_component<Rotation>();
        register_component<Scale>();
        register_component<Player>();
        register_component<Velocity>();

        return true;
    }

    void SceneModule::on_engine_starts_closing()
    {
        m_scenes.clear();
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

            if (const auto it = m_registered_components_types.find(type); it != m_registered_components_types.end())
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
        for (const auto& type : m_registered_components_types | std::views::keys)
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
            if (const auto it = m_registered_components_types.find(type); it != m_registered_components_types.end())
                it->second.serialize(registry, entity, writer);
        }
    }

    void SceneModule::deserialize_component(
        entt::registry& registry,
        const entt::entity entity,
        const entt::id_type type,
        net_utils::Reader& reader) const
    {
        if (const auto it = m_registered_components_types.find(type); it != m_registered_components_types.end())
            it->second.deserialize(registry, entity, reader);
    }

    void SceneModule::serialize_dirty_components(entt::registry& registry, const entt::id_type type, std::vector<u32>& out_entity_ids, net_utils::Writer& writer) const
    {
        if (const auto it = m_registered_components_types.find(type); it != m_registered_components_types.end())
            it->second.serialize_dirty_components(registry, out_entity_ids, writer);
    }

    void SceneModule::for_each_component_type(const std::function<void(entt::id_type)>& function) const
    {
        for (const auto& type : m_registered_components_types | std::views::keys)
            function(type);
    }
}
