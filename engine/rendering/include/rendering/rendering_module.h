#pragma once

#include "core/module.h"

#include <memory>

namespace engine
{
    class IRenderer;

    class RenderingModule : public Module
    {
    public:

        static std::shared_ptr<IRenderer> create_renderer();
        
        // IModule interface
        virtual bool init() override;
        virtual void cleanup() override;
        virtual std::string get_module_name() const override { return "Rendering"; }
        virtual std::vector<std::type_index> get_dependencies() const override;

    };
}