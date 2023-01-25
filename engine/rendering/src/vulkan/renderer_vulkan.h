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
    class Device_Vulkan;
    class Surface_Vulkan;
    class Swapchain_Vulkan;

    class Renderer_Vulkan : public IRenderer
    {
    public:

        Renderer_Vulkan(const GameInfo& game_info, const IWindow& window);
        ~Renderer_Vulkan();
        
        Renderer_Vulkan(const Renderer_Vulkan&) = delete;
        Renderer_Vulkan(Renderer_Vulkan&&) = delete;

        // IRenderer interface
        virtual void       draw_frame()            override;
        virtual IPipeline& create_pipeline() const override;

        // Vulkan wrapper getters
        // Get the renderer's swapchain instance. Must be called after the swapchain is initialized.
        const Surface_Vulkan&   surface()   const;
        const Swapchain_Vulkan& swapchain() const;
        const Device_Vulkan&    device()    const;

        // Native Vulkan getters
        const vk::Instance& instance() const { return m_instance; }

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
        void create_image_views();
        void create_render_pass();
        void create_framebuffers();
        void create_command_pool();
        void create_command_buffers();
        void create_sync_objects();

        void recreate_swapchain();

        std::vector<const char*> get_required_instance_extensions() const;
        bool validation_layers_are_supported() const;

        vk::SurfaceFormatKHR choose_swap_surface_format(const std::vector<vk::SurfaceFormatKHR>& available_formats) const;
        vk::PresentModeKHR choose_swap_present_mode(const std::vector<vk::PresentModeKHR>& available_present_modes) const;
        vk::Extent2D choose_swap_extent(const IWindow& window, const vk::SurfaceCapabilitiesKHR& capabilities) const;

        void record_command_buffer(const vk::CommandBuffer& command_buffer, u32 image_index) const;

    private:

        vk::Instance    m_instance     = nullptr;
        vk::CommandPool m_command_pool = nullptr;

        // Allows the program to start rendering the next frame while the current frame is still drawing.
        // 3 or more frames in flight could add latency, so 2 is good
        static constexpr u32 max_frames_in_flight = 2;
        u32 m_current_frame = 0;

        // These will probably be need to be grouped together in some wrapper class
        // Their size is always max_frames_in_flight, so maybe an std::array would be better
        std::vector<vk::CommandBuffer> m_command_buffers;
        std::vector<vk::Semaphore>     m_image_available_semaphores;
        std::vector<vk::Semaphore>     m_render_finished_semaphores;
        std::vector<vk::Fence>         m_in_flight_fences;

        std::unique_ptr<Device_Vulkan>         m_device          = nullptr;
        std::unique_ptr<DebugMessenger_Vulkan> m_debug_messenger = nullptr;
        std::unique_ptr<Swapchain_Vulkan>      m_swapchain       = nullptr;
        std::unique_ptr<Surface_Vulkan>        m_surface         = nullptr;
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

        bool enable_validation_layers() const { return m_enable_validation_layers; }

        const std::vector<const char*>& validation_layers() const { return m_validation_layers; }

    };
}