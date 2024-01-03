#pragma once

#include "core/module.h"
#include "core/tickable.h"
#include "ui_types.h"

#include <functional>
#include <memory>

namespace h2o
{
    class IUIRenderer;

    class UIModule
        : public Tickable
        , public IModule
    {
    public:

        UIModule();

        void window(
            const std::string& title,
            const ui::Rect& rect,
            const std::function<void(IUIRenderer&)>& window_contents);

        // IModule interface
        bool init(Engine& engine) override;
        void cleanup() override;
        [[nodiscard]] std::string_view module_name() const override { return "h2o_ui"; }
        [[nodiscard]] std::vector<std::type_index> dependencies() const override;

    private:

        std::shared_ptr<IUIRenderer> m_ui_renderer = nullptr;

    };
}