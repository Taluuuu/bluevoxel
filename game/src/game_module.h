#pragma once

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
    class Scene;
}

namespace game
{
    class GameModule : public h2o::IModule
    {
    public:

        GameModule() = default;
        ~GameModule() override = default;

        // h2o::Module interface
        bool init(h2o::Engine& engine) override;
        [[nodiscard]] std::string_view module_name() const override { return "game"; }
        [[nodiscard]] std::vector<std::type_index> dependencies() const override;

    private:

        std::shared_ptr<h2o::Scene> m_scene = nullptr;


        std::shared_ptr<h2o::gfx::Camera>       m_camera       = nullptr;

        h2o::gfx::IRenderer* m_renderer = nullptr;

    };
}