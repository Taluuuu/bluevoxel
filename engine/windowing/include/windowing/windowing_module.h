#pragma once

#include "core/module.h"
#include "core/types.h"

#include <memory>

namespace engine
{
    class IWindow;

    class WindowingModule : public Module
    {
    public:

        /**
         * @brief Create a window
         * 
         * @param width The window's initial width
         * @param height The window's initial height
         * @param title The window's title
         * @return A shared_ptr to the window or nullptr if creation failed
         */
        std::shared_ptr<IWindow> create_window(u32 width, u32 height, const std::string& title);

        // IModule interface
        virtual bool init() override;
        virtual void cleanup() override;
        virtual std::string get_module_name() const override { return "Windowing"; }
        virtual std::vector<std::type_index> get_dependencies() const override { return {}; }

    };
}