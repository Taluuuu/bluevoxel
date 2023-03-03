#pragma once

#include "core/core_interfaces.h"
#include "core/module.h"

#include <memory>

namespace h2o
{
    namespace gfx { class IRenderer; };

    class RenderingModule
        : public Module
        , public ITickable
    {
    public:

        explicit RenderingModule(Engine& engine);

        void tick(f64 delta_time) override;
        
        // IModule interface
        bool init(const GameInfo& game_info) override;
        void cleanup() override;
        [[nodiscard]] std::string_view module_name() const override { return "Rendering"; }
        [[nodiscard]] std::vector<std::type_index> dependencies() const override;

        [[nodiscard]] gfx::IRenderer& renderer() const;

    private:

        std::unique_ptr<gfx::IRenderer> m_renderer;

    };
}