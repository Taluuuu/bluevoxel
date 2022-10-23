#include "windowing/windowing_module.h"

#include "window_glfw.h"

#include <GLFW/glfw3.h>

namespace engine
{
    std::shared_ptr<IWindow> WindowingModule::create_window(u32 width, u32 height, const std::string& title)
    {
        assert(is_initialized());

        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
        glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
        auto handle = glfwCreateWindow(width, height, title.c_str(), NULL, NULL);

        if (!handle)
            return nullptr;

        return std::make_shared<Window_GLFW>(handle);
    }

    bool WindowingModule::init()
    {
        Module::init();
        return glfwInit() == GLFW_TRUE;
    }

    void WindowingModule::cleanup()
    {
        Module::cleanup();
        glfwTerminate();
    }
}