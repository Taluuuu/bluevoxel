#pragma once

#include "rendering/renderer.h"

#include <vector>
#include <vulkan/vulkan.h>

namespace engine
{
    struct AppInfo;
    class IWindow;

    class Renderer_Vulkan : public IRenderer
    {
    public:

        Renderer_Vulkan(const AppInfo& app_info, const IWindow& window);
        ~Renderer_Vulkan();
        
        Renderer_Vulkan(const Renderer_Vulkan&) = delete;
        Renderer_Vulkan(Renderer_Vulkan&&) = delete;

    private:

        void setup_debug_messenger();

    private:

        VkInstance m_instance = nullptr;

#ifdef NDEBUG
        constexpr static bool enable_validation_layers = false;
#else
        constexpr static bool enable_validation_layers = true;
#endif

        const std::vector<const char*> m_validation_layers = 
        {
            "VK_LAYER_KHRONOS_validation"
        };

        VkDebugUtilsMessengerEXT m_debug_messenger;

    };
}