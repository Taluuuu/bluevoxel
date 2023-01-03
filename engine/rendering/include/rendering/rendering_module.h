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

        RenderingModule(Engine& engine);

        virtual void tick(f64 delta_time) override;
        
        // IModule interface
        virtual bool init(const GameInfo& game_info) override;
        virtual void cleanup() override;
        virtual std::string_view get_module_name() const override { return "Rendering"; }
        virtual std::vector<std::type_index> get_dependencies() const override;

        IRenderer& renderer() const;

    private:

        std::shared_ptr<IRenderer> m_renderer = nullptr;

    };
}