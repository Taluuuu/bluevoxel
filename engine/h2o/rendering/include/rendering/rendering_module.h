#pragma once

#include "core/tickable.h"
#include "core/module.h"

#include <memory>

namespace h2o
{
    namespace gfx { class IRenderer; };

    class RenderingModule
        : public Tickable
        , public IModule
    {
    public:

        RenderingModule() = default;
        ~RenderingModule() override = default;

        // Tickable interface
        void pre_render(f32 delta_time) override;

        // IModule interface
        bool init(Engine& engine) override;
        [[nodiscard]] std::string_view module_name() const override { return "h2o_rendering"; }
        [[nodiscard]] std::vector<std::type_index> dependencies() const override;

        [[nodiscard]] gfx::IRenderer& renderer() const;

    private:

        std::unique_ptr<gfx::IRenderer> m_renderer;

    };
}