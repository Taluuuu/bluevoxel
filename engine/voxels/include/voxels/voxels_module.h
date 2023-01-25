#pragma once

#include "core/module.h"

namespace engine
{
    class VoxelsModule : public Module
    {
    public:

        VoxelsModule(Engine& engine);

        // Module interface
        virtual std::string_view get_module_name() const override;
        virtual std::vector<std::type_index> get_dependencies() const override;
        
    };
}