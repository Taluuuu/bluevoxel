#pragma once

#include "core/module.h"
#include "core/tickable.h"
#include "networking/networking_utils.h"

#include <entt/core/fwd.hpp>
#include <entt/entity/registry.hpp>
#include <set>

namespace h2o
{
    class Scene;

    class SceneModule : public IModule
    {
    public:

        // IModule interface
        [[nodiscard]] bool init(Engine& engine) override;
        [[nodiscard]] std::string_view module_name() const override { return "h2o_scene"; }
        void on_engine_starts_closing() override;

        void register_scene(Scene& scene);
        void unregister_scene(Scene& scene);

        [[nodiscard]] entt::entity deserialize_entity(entt::registry& registry, net_utils::Reader& reader) const;
        void serialize_entity(const entt::registry& registry, entt::entity entity, net_utils::Writer& writer) const;

        template<class T>
        void register_component();

    private:

        std::set<Scene*> m_scenes{};

        struct ReplicatedComponent
        {
            void (*serialize)(const entt::registry&, entt::entity, net_utils::Writer&);
            void (*deserialize)(entt::registry&, entt::entity, net_utils::Reader&);
        };

        std::unordered_map<entt::id_type, ReplicatedComponent> m_registered_components{};

    };

    template<class T>
    void SceneModule::register_component()
    {
        const auto type = entt::type_hash<T>::value();
        m_registered_components[type] = ReplicatedComponent
        {
            .serialize =
                [](const entt::registry& registry, entt::entity entity, net_utils::Writer& writer)
                {
                    writer.object(registry.get<T>(entity));
                },
            .deserialize =
                [](entt::registry& registry, entt::entity entity, net_utils::Reader& reader)
                {
                    if (!registry.any_of<T>(entity))
                        registry.emplace<T>(entity);

                    reader.object(registry.get<T>(entity));
                }
        };
    }
}
