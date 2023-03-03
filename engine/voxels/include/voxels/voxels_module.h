#pragma once

#include "core/module.h"

namespace h2o::vxl
{
    class VoxelsModule : public Module
    {
    public:

        explicit VoxelsModule(Engine& engine);

        // Module interface
        [[nodiscard]] std::string_view module_name() const override;
        [[nodiscard]] std::vector<std::type_index> dependencies() const override;
        
    };
}