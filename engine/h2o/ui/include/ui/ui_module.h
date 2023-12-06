#pragma once

#include "core/module.h"
#include "core/tickable.h"

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

        [[nodiscard]] IUIRenderer& ui() const;

        // IModule interface
        bool init(Engine& engine) override;
        void cleanup() override;
        [[nodiscard]] std::string_view module_name() const override { return "h2o_ui"; }
        [[nodiscard]] std::vector<std::type_index> dependencies() const override;

    private:

        std::shared_ptr<IUIRenderer> m_ui_renderer = nullptr;

    };
}