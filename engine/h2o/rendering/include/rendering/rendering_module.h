#pragma once

#include "core/tickable.h"
#include "core/module.h"

#include <memory>

namespace h2o
{
    namespace gfx
    {
        class IRenderer;
        class Renderer_Base;
    };

    class RenderingModule
        : public Tickable
        , public IModule
    {
    public:

        RenderingModule();
        ~RenderingModule() override = default;

        // IModule interface
        bool init(Engine& engine) override;
        [[nodiscard]] std::string_view module_name() const override { return "h2o_rendering"; }
        [[nodiscard]] std::vector<std::type_index> dependencies() const override;

        // Tickable interface
        void frame_start(f32 delta_time) override;
        void frame_end(f32 delta_time) override;

        // High level renderer, meant for user
        [[nodiscard]] gfx::IRenderer& renderer() const;
        // Lower level renderer
        [[nodiscard]] gfx::Renderer_Base& renderer_base() const;

    private:

        std::unique_ptr<gfx::Renderer_Base> m_renderer;

    };
}