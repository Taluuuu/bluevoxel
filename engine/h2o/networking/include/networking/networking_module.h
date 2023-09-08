#pragma once

#include "core/module.h"

namespace h2o
{
    class NetworkingModule : public IModule
    {
    public:

        ~NetworkingModule() override = default;

        // IModule interface
        [[nodiscard]] bool init(Engine& engine) override;
        void cleanup() override;
        [[nodiscard]] std::string_view module_name() const override { return "h2o_networking_module"; }
        [[nodiscard]] std::vector<std::type_index> dependencies() const override;

    };
}