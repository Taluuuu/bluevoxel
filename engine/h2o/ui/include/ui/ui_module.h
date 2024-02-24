#pragma once

#include "core/module.h"
#include "core/tickable.h"
#include "ui_types.h"

#include <functional>
#include <memory>

namespace h2o
{
    class InputModule;
    class IUIRenderer;

    class UIModule
        : public Tickable
        , public IModule
    {
    public:

        UIModule();

        [[nodiscard]] static bool is_interacting_with_ui();

        // IModule interface
        bool init(Engine& engine) override;
        [[nodiscard]] std::string_view module_name() const override { return "h2o_ui"; }
        [[nodiscard]] std::vector<std::type_index> dependencies() const override;

    protected:

        // Tickable interface
        void frame_end(f32 delta_time) override;

    private:

        std::shared_ptr<IUIRenderer> m_ui_renderer = nullptr;

        InputModule* m_input_module = nullptr;

    };
}