#pragma once

#include "core/core_interfaces.h"
#include "core/module.h"

#include <memory>

namespace h2o
{
    namespace gfx { class IRenderer; };

    class RenderingModule
        : public IModule
        , public ITickable
    {
    public:

        RenderingModule() = default;
        ~RenderingModule() override = default;

        // ITickable interface
        void tick(TickPhase phase, f64 delta_time) override;
        
        // IModule interface
        bool init(Engine& engine) override;
        [[nodiscard]] std::string_view module_name() const override { return "h2o_rendering"; }
        [[nodiscard]] std::vector<std::type_index> dependencies() const override;

        [[nodiscard]] gfx::IRenderer& renderer() const;

    private:

        std::unique_ptr<gfx::IRenderer> m_renderer;

    };
}