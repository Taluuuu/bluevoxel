#pragma once

#include "core/core_interfaces.h"
#include "core/module.h"

#include <memory>

// Forward declarations
namespace h2o
{
    namespace gfx
    {
        class Camera;
        class IRenderer;
        class IPipeline;
        class IVertexArray;
    }

    class InputModule;
}

namespace game
{
    class GameModule
        : public h2o::IModule
        , public h2o::ITickable
    {
    public:

        GameModule() = default;
        ~GameModule() override = default;

        // h2o::Module interface
        bool init(h2o::Engine& engine) override;
        [[nodiscard]] std::string_view module_name() const override { return "game_module"; }
        [[nodiscard]] std::vector<std::type_index> dependencies() const override;

        // h2o::ITickable interface
        void tick(h2o::TickPhase phase, f64 delta_time) override;

    private:

        std::shared_ptr<h2o::gfx::IPipeline>    m_pipeline     = nullptr;
        std::shared_ptr<h2o::gfx::IVertexArray> m_vertex_array = nullptr;
        std::shared_ptr<h2o::gfx::Camera>       m_camera       = nullptr;

        h2o::gfx::IRenderer* m_renderer = nullptr;

        h2o::InputModule* m_input_module = nullptr;

    };
}