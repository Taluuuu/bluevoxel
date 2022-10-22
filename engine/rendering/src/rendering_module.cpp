#include "rendering/rendering_module.h"

#include <vulkan/vulkan.h>

namespace engine
{
    std::shared_ptr<IRenderer> RenderingModule::create_renderer()
    {
        return nullptr;
    }

    bool RenderingModule::init_impl()
    {
        VkApplicationInfo app_info
        {
            .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pApplicationName = "Hello Triangle",
            .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
            .pEngineName = "No Engine",
            .engineVersion = VK_MAKE_VERSION(1, 0, 0),
            .apiVersion = VK_API_VERSION_1_0,
        };

        VkInstanceCreateInfo create_info
        {
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pApplicationInfo = &app_info,
        };

        return true;
    }

    void RenderingModule::cleanup_impl()
    {

    }
}