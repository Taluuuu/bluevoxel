#pragma once

#include "core/core_interfaces.h"
#include "core/module.h"

#include <memory>

namespace engine
{
    class IRenderer;
    class IPipeline;
    class IVertexArray;
}

namespace game
{
    class GameModule
        : public engine::Module
        , public engine::ITickable
    {
    public:

        explicit GameModule(engine::Engine& engine);

        // engine::Module interface
        bool init(const engine::GameInfo& game_info) override;
        [[nodiscard]] std::string_view get_module_name() const override;
        [[nodiscard]] std::vector<std::type_index> get_dependencies() const override;

        // engine::ITickable interface
        void tick(f64 delta_time) override;

    private:

        std::shared_ptr<engine::IPipeline>    m_pipeline     = nullptr;
        std::shared_ptr<engine::IVertexArray> m_vertex_array = nullptr;

        engine::IRenderer* m_renderer = nullptr;

    };
}