#pragma once

#include "core/types.h"
#include "rendering/renderer.h"

#include <memory>
#include <optional>
#include <vulkan/vulkan.hpp>

namespace engine
{
    struct GameInfo;
    class IWindow;

    class DebugMessenger_Vulkan;
    class Pipeline_Vulkan;
    class PhysicalDevice_Vulkan;
    class Swapchain_Vulkan;

    class Renderer_Vulkan : public IRenderer
    {
    public:

        Renderer_Vulkan(const GameInfo& game_info, const IWindow& window);
        ~Renderer_Vulkan();
        
        Renderer_Vulkan(const Renderer_Vulkan&) = delete;
        Renderer_Vulkan(Renderer_Vulkan&&) = delete;

        // IRenderer interface
        virtual void draw_frame() const override;
        virtual IPipeline& create_pipeline() const override;

        // Vulkan wrapper getters
        // Get the renderer's swapchain instance. Must be called after the swapchain is initialized.
        const Swapchain_Vulkan& swapchain() const;

        // Native Vulkan getters
        const vk::Instance&   instance()    const { return m_instance;    }
        const vk::SurfaceKHR& surface()     const { return m_surface;     }
        const vk::Device&     device()      const { return m_device;      }
        const vk::RenderPass& render_pass() const { return m_render_pass; }

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

        void create_instance(const char* game_name, const char* engine_name);
        void create_surface(const IWindow& window);
        void create_logical_device();
        void create_image_views();
        void create_render_pass();
        void create_framebuffers();
        void create_command_pool();
        void create_command_buffer();
        void create_sync_objects();

        std::vector<const char*> get_required_instance_extensions() const;
        bool validation_layers_are_supported() const;

        vk::SurfaceFormatKHR choose_swap_surface_format(const std::vector<vk::SurfaceFormatKHR>& available_formats) const;
        vk::PresentModeKHR choose_swap_present_mode(const std::vector<vk::PresentModeKHR>& available_present_modes) const;
        vk::Extent2D choose_swap_extent(const IWindow& window, const vk::SurfaceCapabilitiesKHR& capabilities) const;

        void record_command_buffer(const vk::CommandBuffer& command_buffer, u32 image_index) const;

    private:

        vk::Instance                 m_instance                  = nullptr;
        vk::SurfaceKHR               m_surface                   = nullptr;

        vk::Device                   m_device                    = nullptr;

        vk::Queue                    m_graphics_queue            = nullptr;
        vk::Queue                    m_present_queue             = nullptr;

        std::vector<vk::ImageView>   m_swapchain_image_views;
        std::vector<vk::Framebuffer> m_swapchain_framebuffers;

        vk::RenderPass               m_render_pass               = nullptr;

        vk::CommandPool              m_command_pool              = nullptr;
        vk::CommandBuffer            m_command_buffer            = nullptr;

        vk::Semaphore                m_image_available_semaphore = nullptr;
        vk::Semaphore                m_render_finished_semaphore = nullptr;
        vk::Fence                    m_in_flight_fence           = nullptr;

        std::unique_ptr<PhysicalDevice_Vulkan> m_physical_device = nullptr;
        std::unique_ptr<DebugMessenger_Vulkan> m_debug_messenger = nullptr;
        std::unique_ptr<Swapchain_Vulkan>      m_swapchain       = nullptr;
        std::unique_ptr<Pipeline_Vulkan>       m_pipeline        = nullptr;

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

    public:

        const std::vector<const char*>& device_extensions() const { return m_device_extensions; }

    };
}