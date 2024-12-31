#pragma once

#include "core/core_interfaces.h"
#include "core/module.h"
#include "core/tickable.h"
#include "core/types.h"
#include "window.h"

#include <memory>

namespace h2o
{
    class IWindow;

    class WindowingModule
        : public Tickable
        , public IModule
        , public ITimeProvider
    {
    public:

        WindowingModule();
        ~WindowingModule() override = default;

        [[nodiscard]] IWindow& window() const;

        // IModule interface
        bool init(Engine& engine) override;
        void cleanup() override;
        [[nodiscard]] std::string_view module_name() const override { return "h2o_windowing"; }

        // IDeltaTimeProvider interface
        [[nodiscard]] f64 delta_time() const override;
        [[nodiscard]] f64 time() const override;

    protected:

        // Tickable interface
        void frame_start() override;
        void frame_end(f32 delta_time) override;

    private:

        std::unique_ptr<IWindow> m_window = nullptr;

        static constexpr v2i default_size { 1600, 900 };

    };
}
