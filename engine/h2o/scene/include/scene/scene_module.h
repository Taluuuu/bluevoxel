#pragma once

#include "core/module.h"
#include "core/tickable.h"

#include <entt/core/fwd.hpp>
#include <entt/entity/registry.hpp>
#include <bitsery/adapter/buffer.h>
#include <bitsery/bitsery.h>
#include <bitsery/brief_syntax.h>
#include <bitsery/common.h>
#include <bitsery/ext/entropy.h>
#include <bitsery/ext/pointer.h>
#include <bitsery/ext/utils/pointer_utils.h>
#include <bitsery/traits/vector.h>
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

        template<class T>
        void register_component();

    private:

        std::set<Scene*> m_scenes{};

        using Buffer = std::vector<u8>;
        using Reader = bitsery::InputBufferAdapter<Buffer>;
        using Writer = bitsery::OutputBufferAdapter<Buffer>;
        struct ReplicatedComponent
        {
            void (*serialize)(entt::registry&, entt::entity, bitsery::Serializer<Writer>&);
            void (*deserialize)(entt::registry&, entt::entity, bitsery::Deserializer<Reader>&);
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
                [](entt::registry& registry, entt::entity entity, bitsery::Serializer<Writer>& writer)
                {
                    writer.object(registry.get<T>(entity));
                },
            .deserialize =
                [](entt::registry& registry, entt::entity entity, bitsery::Deserializer<Reader>& reader)
                {
                    if (!registry.any_of<T>(entity))
                        registry.emplace<T>(entity);

                    reader.object(registry.get<T>(entity));
                }
        };
    }
}
