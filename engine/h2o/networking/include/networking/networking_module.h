#pragma once

#include "core/events.h"
#include "core/module.h"
#include "networking_types.h"

#include <set>
#include <unordered_map>

namespace h2o
{
    class NetPeer;

    class NetworkingModule : public IModule
    {
    public:

        ~NetworkingModule() override = default;

        // IModule interface
        [[nodiscard]] bool init(Engine& engine) override;
        void cleanup() override;
        [[nodiscard]] std::string_view module_name() const override { return "h2o_networking"; }
        [[nodiscard]] std::vector<std::type_index> dependencies() const override;

    };
}