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
        void create_swapchain(const IWindow& window);

        std::vector<const char*> get_required_instance_extensions() const;
        bool validation_layers_are_supported() const;
        void populate_debug_messenger_create_info(vk::DebugUtilsMessengerCreateInfoEXT& debug_utils_messenger_create_info) const;
        bool is_device_suitable(const vk::PhysicalDevice& device) const;
        bool device_supports_extensions(const vk::PhysicalDevice& device) const;

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

        struct SwapChainSupportDetails
        {
            vk::SurfaceCapabilitiesKHR capabilities;
            std::vector<vk::SurfaceFormatKHR> formats;
            std::vector<vk::PresentModeKHR> present_modes;

            bool is_adequate() const
            {
                return
                    !formats.empty() &&
                    !present_modes.empty();
            }
        };

        SwapChainSupportDetails query_swapchain_support(const vk::PhysicalDevice& device) const;
        vk::SurfaceFormatKHR choose_swap_surface_format(const std::vector<vk::SurfaceFormatKHR>& available_formats) const;
        vk::PresentModeKHR choose_swap_present_mode(const std::vector<vk::PresentModeKHR>& available_present_modes) const;
        vk::Extent2D choose_swap_extent(const IWindow& window, const vk::SurfaceCapabilitiesKHR& capabilities) const;

    private:

        vk::Instance               m_instance               = nullptr;
        vk::DebugUtilsMessengerEXT m_debug_messenger        = nullptr;
        vk::SurfaceKHR             m_surface                = nullptr;

        vk::PhysicalDevice         m_physical_device        = nullptr;
        vk::Device                 m_device                 = nullptr;

        vk::Queue                  m_queue                  = nullptr;
        vk::Queue                  m_present_queue          = nullptr;

        vk::SwapchainKHR           m_swapchain              = nullptr;
        std::vector<vk::Image>     m_swapchain_images;
        vk::Format                 m_swapchain_image_format = nullptr;
        vk::Extent2D               m_swapchain_extent       = nullptr;

    private:

        // TODO: Make below constexpr
        const std::vector<const char*> m_device_extensions =
        {
            VK_KHR_SWAPCHAIN_EXTENSION_NAME
        };

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