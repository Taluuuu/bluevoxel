#pragma once

#include "core/core_interfaces.h"
#include "core/events.h"
#include "core/module.h"
#include "core/types.h"
#include "window.h"

#include <memory>

namespace h2o
{
    class IWindow;

    class WindowingModule
        : public IModule
        , public IWindowModule
    {
    public:

        WindowingModule() = default;
        ~WindowingModule() override = default;

        // IModule interface
        bool init(Engine& engine) override;
        void cleanup() override;
        [[nodiscard]] std::string_view module_name() const override { return "h2o_windowing"; }
        [[nodiscard]] std::vector<std::type_index> dependencies() const override { return {}; }

        // IWindowModule interface
        [[nodiscard]] f64 delta_time() const override;
        void poll_events() const override;
        void swap_buffers(f64 max_fps) const override;

        [[nodiscard]] IWindow& window() const;

    private:

        std::unique_ptr<IWindow> m_window = nullptr;

        static constexpr v2i default_size { 800, 600 };

    };
}