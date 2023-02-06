#pragma once

#include "core/core_interfaces.h"
#include "core/module.h"

namespace game
{
    class GameModule
        : public engine::Module
        , public engine::ITickable
    {
    public:

        GameModule(engine::Engine& engine);

        // engine::Module interface
        virtual bool init(const engine::GameInfo& game_info) override;
        virtual std::string_view get_module_name() const override;
        virtual std::vector<std::type_index> get_dependencies() const override;

        // engine::ITickable interface
        virtual void tick(f64 delta_time) override;

    };
}