#pragma once

#include "core/core_interfaces.h"
#include "core/module.h"

#include <memory>

namespace engine
{
    class IRenderer;

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
        [[nodiscard]] std::string_view get_module_name() const override { return "Rendering"; }
        [[nodiscard]] std::vector<std::type_index> get_dependencies() const override;

        [[nodiscard]] IRenderer& renderer() const;

    private:

        std::unique_ptr<IRenderer> m_renderer;

    };
}