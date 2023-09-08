#pragma once

#include "core/module.h"
#include "core/tickable.h"

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
    class Scene;
}

namespace game
{
    class GameModule
        : public h2o::IModule
        , public h2o::Tickable
    {
    public:

        GameModule() = default;
        ~GameModule() override = default;

        // h2o::IModule interface
        bool init(h2o::Engine& engine) override;
        [[nodiscard]] std::string_view module_name() const override { return "game"; }
        [[nodiscard]] std::vector<std::type_index> dependencies() const override;

        // h2o::Tickable interface
        void update(f32 delta_time) override;

    private:

        std::shared_ptr<h2o::Scene> m_scene = nullptr;

        h2o::gfx::IRenderer* m_renderer = nullptr;

    };
}