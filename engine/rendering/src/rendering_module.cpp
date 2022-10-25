#include "rendering/rendering_module.h"

#include "windowing/windowing_module.h"

#include <typeinfo>
#include <vulkan/vulkan.h>

namespace engine
{
    RenderingModule::RenderingModule(Engine& engine)
        : Module(engine) {}

    bool RenderingModule::init(const AppInfo& app_info)
    {
        Module::init(app_info);
        VkApplicationInfo vulkan_app_info
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
            .pApplicationInfo = &vulkan_app_info,
        };

        return true;
    }

    void RenderingModule::cleanup()
    {

    }

    std::vector<std::type_index> RenderingModule::get_dependencies() const
    {
        return { typeid(WindowingModule) };
    }
}