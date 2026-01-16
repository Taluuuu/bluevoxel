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
        entt::meta_factory<NetworkSync>()
            .type("NetworkSync"_hs)
            .data<&NetworkSync::owner>("owner"_hs);
        register_component<NetworkSync>();

        entt::registry test_reg;
        Buffer buffer;

        {
            const auto test_ent = test_reg.create();
            test_reg.emplace<NetworkSync>(test_ent, 6);

            log::info("{}", test_reg.get<NetworkSync>(test_ent).owner);

            std::vector<entt::id_type> component_types{};
            for (const auto& type : m_registered_components | std::views::keys)
            {
                if (const auto storage = test_reg.storage(type))
                {
                    if (storage->contains(test_ent))
                        component_types.emplace_back(type);
                }
            }

            bitsery::Serializer<Writer> writer{ buffer };
            writer.value8b(component_types.size());
            for (const auto& type : component_types)
            {
                writer.value4b(type);
                m_registered_components[type].serialize(test_reg, test_ent, writer);
            }
        }

        {
            const auto test_ent = test_reg.create();
            bitsery::Deserializer<Reader> reader{ buffer.begin(), buffer.size() };

            u64 num_components{};
            reader.value8b(num_components);

            for (u64 i = 0; i < num_components; ++i)
            {
                entt::id_type component_type{};
                reader.value4b(component_type);
                m_registered_components[component_type].deserialize(test_reg, test_ent, reader);
            }

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
}
