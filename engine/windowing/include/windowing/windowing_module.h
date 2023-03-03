#pragma once

#include "core/events.h"
#include "core/module.h"
#include "core/types.h"
#include "core/core_interfaces.h"

#include <memory>

namespace h2o
{
    class IWindow;

    class WindowingModule
        : public Module
        , public IWindowModule
    {
    public:

        WindowingModule(Engine& engine);

        // Module virtual methods
        virtual bool init(const GameInfo& game_info) override;
        virtual void cleanup() override;
        virtual std::string_view module_name() const override { return "Windowing"; }
        virtual std::vector<std::type_index> dependencies() const override { return {}; }

        // ICoreWindow interface
        virtual f64 delta_time() const override;
        virtual bool should_close() const override;
        virtual void poll_events() const override;
        virtual void swap_buffers(f64 max_fps) const override;

        IWindow& window() const;

    private:

        std::shared_ptr<IWindow> m_window = nullptr;

        static constexpr v2i default_size = { 800, 600 };

    };
}