#pragma once

#include "core/core_interfaces.h"
#include "core/module.h"

#include <memory>

namespace h2o::gfx
{
    class Camera;
    class IRenderer;
    class IPipeline;
    class IVertexArray;
}

namespace game
{
    class GameModule
        : public h2o::Module
        , public h2o::ITickable
    {
    public:

        explicit GameModule(h2o::Engine& engine);

        // h2o::Module interface
        bool init(const h2o::GameInfo& game_info) override;
        [[nodiscard]] std::string_view module_name() const override;
        [[nodiscard]] std::vector<std::type_index> dependencies() const override;

        // h2o::ITickable interface
        void tick(f64 delta_time) override;

    private:

        std::shared_ptr<h2o::gfx::IPipeline>    m_pipeline     = nullptr;
        std::shared_ptr<h2o::gfx::IVertexArray> m_vertex_array = nullptr;
        std::shared_ptr<h2o::gfx::Camera>       m_camera       = nullptr;

        h2o::gfx::IRenderer* m_renderer = nullptr;

    };
}