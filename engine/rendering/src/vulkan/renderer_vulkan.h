#pragma once

#include "rendering/renderer.h"

#include <vulkan/vulkan.hpp>

namespace engine
{
    struct GameInfo;
    class IWindow;

    class Renderer_Vulkan : public IRenderer
    {
    public:

        Renderer_Vulkan(const GameInfo& game_info, const IWindow& window);
        ~Renderer_Vulkan();
        
        Renderer_Vulkan(const Renderer_Vulkan&) = delete;
        Renderer_Vulkan(Renderer_Vulkan&&) = delete;

    private:

        void create_instance(const char* game_name, const char* engine_name);
        void setup_debug_messenger();

        std::vector<const char*> get_required_extensions() const;
        bool validation_layers_are_supported() const;
        void populate_debug_messenger_create_info(vk::DebugUtilsMessengerCreateInfoEXT& debug_utils_messenger_create_info) const;

    private:

        vk::Instance m_instance;
        vk::DebugUtilsMessengerEXT m_debug_messenger;

        const std::vector<const char*> m_validation_layers = 
        {
            "VK_LAYER_KHRONOS_validation"
        };

#ifdef NDEBUG
        const bool m_enable_validation_layers = false;
#else
        const bool m_enable_validation_layers = true;
#endif

    };
}