#pragma once

#include "core/module.h"
#include "core/types.h"

#include <memory>
#include <string>

namespace engine
{
    class IWindow;

    class WindowingModule : public Module<WindowingModule>
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
        static std::shared_ptr<IWindow> create_window(u32 width, u32 height, const std::string& title);

    protected:

        friend class Module<WindowingModule>;

        static bool init_impl();
        static void cleanup_impl();

    };
}