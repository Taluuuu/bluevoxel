#pragma once

#include "core/module.h"
#include "core/tickable.h"
#include "networking/networking_utils.h"
#include "scene_networking_components.h"

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

        entt::entity deserialize_entity(entt::registry& registry, net_utils::Reader& reader) const;
        void serialize_entity(const entt::registry& registry, entt::entity entity, net_utils::Writer& writer) const;
        void deserialize_component(entt::registry& registry, entt::entity entity, entt::id_type type, net_utils::Reader& reader, bool mark_dirty) const;
        // Removes Dirty<T> components from the registry
        void serialize_dirty_components(entt::registry& registry, entt::id_type type, std::vector<u32>& out_entity_ids, net_utils::Writer& writer) const;

        template<class T>
        void register_component();

        void for_each_component_type(const std::function<void(entt::id_type)>& function) const;

    private:

        std::set<Scene*> m_scenes{};

        struct ReplicatedComponent
        {
            std::function<void(entt::registry&, std::vector<u32>&, net_utils::Writer&)> serialize_dirty_components;
            std::function<void(const entt::registry&, entt::entity, net_utils::Writer&)> serialize;
            std::function<void(entt::registry&, entt::entity, net_utils::Reader&, bool)> deserialize;
        };

        std::unordered_map<entt::id_type, ReplicatedComponent> m_registered_components_types{};

    };

    template<class T>
    void SceneModule::register_component()
    {
        const auto type = entt::type_hash<T>::value();
        m_registered_components_types[type] = ReplicatedComponent
        {
            .serialize_dirty_components =
                [](entt::registry& registry, std::vector<u32>& out_entity_ids, net_utils::Writer& writer)
                {
                    // TODO: Clean this up
                    if constexpr (std::is_empty_v<T>)
                    {
                        registry.view<T, Dirty<T>, NetworkSync>().each(
                            [&](const entt::entity entity, const NetworkSync& sync)
                            {
                                out_entity_ids.push_back(sync.entity_id);
                                registry.remove<Dirty<T>>(entity);
                            }
                        );
                    }
                    else
                    {
                        registry.view<T, Dirty<T>, NetworkSync>().each(
                            [&](const entt::entity entity, const T& component, const NetworkSync& sync)
                            {
                                out_entity_ids.push_back(sync.entity_id);
                                writer.object(component);
                                registry.remove<Dirty<T>>(entity);
                            }
                        );
                    }
                },
            .serialize =
                [](const entt::registry& registry, const entt::entity entity, net_utils::Writer& writer)
                {
                    if constexpr (!std::is_empty_v<T>)
                        writer.object(registry.get<T>(entity));
                },
            .deserialize =
                [](entt::registry& registry, const entt::entity entity, net_utils::Reader& reader, const bool mark_dirty)
                {
                    if constexpr (std::is_empty_v<T>)
                    {
                        if (!registry.any_of<T>(entity))
                            registry.emplace<T>(entity);
                    }
                    else
                    {
                        if (registry.any_of<T>(entity))
                        {
                            reader.object(registry.get<T>(entity));
                        }
                        else
                        {
                            T comp;
                            reader.object(comp);
                            registry.emplace<T>(entity, comp);
                        }

                        registry.emplace_or_replace<Dirty<T>>(entity);
                    }

                }
        };
    }
}
