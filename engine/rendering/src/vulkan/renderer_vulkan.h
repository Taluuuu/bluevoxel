#pragma once

#include "core/types.h"
#include "rendering/renderer.h"

#include <optional>
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
        void create_surface(const IWindow& window);
        void pick_physical_device();
        void create_logical_device();

        std::vector<const char*> get_required_extensions() const;
        bool validation_layers_are_supported() const;
        void populate_debug_messenger_create_info(vk::DebugUtilsMessengerCreateInfoEXT& debug_utils_messenger_create_info) const;
        bool is_device_suitable(const vk::PhysicalDevice& device) const;

        struct QueueFamilyIndices
        {
            std::optional<u32> graphics_family;
            std::optional<u32> present_family;

            bool is_complete() const
            {
                return 
                    graphics_family.has_value() && 
                    present_family.has_value();
            }
        };

        QueueFamilyIndices find_queue_families(const vk::PhysicalDevice& device) const;

    private:

        vk::Instance               m_instance        = nullptr;
        vk::DebugUtilsMessengerEXT m_debug_messenger = nullptr;
        vk::SurfaceKHR             m_surface         = nullptr;
        vk::PhysicalDevice         m_physical_device = nullptr;
        vk::Device                 m_device          = nullptr;
        vk::Queue                  m_queue           = nullptr;
        vk::Queue                  m_present_queue   = nullptr;

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