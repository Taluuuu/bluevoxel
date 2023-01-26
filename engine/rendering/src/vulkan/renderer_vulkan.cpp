#include "renderer_vulkan.h"

#include "core/game_info.h"
#include "core/types.h"
#include "core/utils.h"
#include "windowing/window.h"

#include "debug_messenger_vulkan.h"
#include "device_vulkan.h"
#include "pipeline_vulkan.h"
#include "shader_vulkan.h"
#include "surface_vulkan.h"
#include "swapchain_vulkan.h"

// GLFW for Vulkan
// TODO: Should not assume Vulkan is used with GLFW.
#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <iostream>
#undef max
#include <limits>
#include <set>

namespace engine
{
    Renderer_Vulkan::Renderer_Vulkan(const GameInfo& game_info, const IWindow& window)
    {
        // TODO: Return raw pointers from my create functions
        // and store them into smart pointers here

        // TODO: Make sure to destroy the instance if setup_debug_messenger fails.
        create_instance(game_info.game_name.data(), game_info.engine_name.data());

        m_debug_messenger.reset(DebugMessenger_Vulkan::create(*this));
        if (!m_debug_messenger)
        {
            throw std::runtime_error("Failed to create debug messenger.");
        }

        m_surface.reset(Surface_Vulkan::create(window, *this));
        if (!m_surface)
        {
            throw std::runtime_error("Failed to create vulkan surface");
        }

        m_device.reset(Device_Vulkan::create(*this));
        if (!m_device)
        {
            throw std::runtime_error("Failed to pick physical device.");
        }

        // TODO: Change this stupid constructor
        m_swapchain.reset(Swapchain_Vulkan::create(*this, window));
        if (!m_swapchain)
        {
            // TODO: Destroy previously allocated resources
            throw std::runtime_error("Failed to create swapchain.");
        }

        // Pipeline creation here is temporary; pipelines will be created in game code
        // or in more abstract mesh renderers in the engine
        m_pipeline = std::make_unique<Pipeline_Vulkan>(*this);
        (*m_pipeline)
            .add_shader(ShaderStage::Vertex,   "Resources/engine/shaders/triangle.vert.spv")
            .add_shader(ShaderStage::Fragment, "Resources/engine/shaders/triangle.frag.spv")
            .compile();
        
        if (!m_pipeline->is_ready())
        {
            // TODO: Destroy previously allocated resources
            throw std::runtime_error("Failed to create graphics pipeline.");
        }

        create_command_pool();

        create_command_buffers();

        create_sync_objects();
    }

    Renderer_Vulkan::~Renderer_Vulkan()
    {
        for (size_t i = 0; i < max_frames_in_flight; i++)
        {
            m_device->handle().destroySemaphore(m_image_available_semaphores[i]);
            m_device->handle().destroySemaphore(m_render_finished_semaphores[i]);
            m_device->handle().destroyFence(m_in_flight_fences[i]);
        }

        m_device->handle().destroyCommandPool(m_command_pool);
        
        m_pipeline.reset();
        m_swapchain.reset();
        m_device->handle().destroy();

        m_surface.reset();
        m_debug_messenger.reset();
        m_instance.destroy();
    }

    void Renderer_Vulkan::draw_frame()
    {
        assert(m_swapchain);

        const auto& image_available_semaphore = m_image_available_semaphores[m_current_frame];
        const auto& render_finished_semaphore = m_render_finished_semaphores[m_current_frame];
        const auto& in_flight_fence = m_in_flight_fences[m_current_frame];
        const auto& command_buffer = m_command_buffers[m_current_frame];

        // TODO: Figure out why nodiscard
        auto wait_fence_result = m_device->handle().waitForFences(in_flight_fence, VK_TRUE, UINT64_MAX);
        m_device->handle().resetFences(in_flight_fence);

        // TODO: Check this .value i'm really fucking tired so i trust future me to do it
        u32 image_index = 
            m_device->handle().acquireNextImageKHR(m_swapchain->handle(), UINT64_MAX, image_available_semaphore, nullptr).value;

        command_buffer.reset();

        // More efficient not to record command buffer every frame
        record_command_buffer(command_buffer, image_index);

        vk::Semaphore wait_semaphores[]   = { image_available_semaphore };
        vk::Semaphore signal_semaphores[] = { render_finished_semaphore };
        vk::PipelineStageFlags wait_stages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
        vk::SubmitInfo submit_info(
            1, wait_semaphores,
            wait_stages,
            1, &command_buffer,
            1, signal_semaphores
        );

        m_device->graphics_queue().submit(submit_info, in_flight_fence);

        vk::PresentInfoKHR present_info(
            1, signal_semaphores,
            1, &m_swapchain->handle(),
            &image_index,
            nullptr
        );

        // TODO: Figure out why nodiscard
        auto present_result = m_device->present_queue().presentKHR(present_info);

        // This may be better elsewhere
        m_device->handle().waitIdle();

        m_current_frame = (m_current_frame + 1) % max_frames_in_flight;
    }

    IPipeline& Renderer_Vulkan::create_pipeline() const
    {
        // TODO: This is very wrong.
        // Pipelines should be stored and returned as a reference.
        return *m_pipeline;
    }

    const Surface_Vulkan& Renderer_Vulkan::surface() const
    {
        assert(m_surface);
        return *m_surface;
    }

    const Swapchain_Vulkan& Renderer_Vulkan::swapchain() const
    {
        assert(m_swapchain);
        return *m_swapchain;
    }

    const Device_Vulkan& Renderer_Vulkan::device() const
    {
        assert(m_device);
        return *m_device;
    }

    void Renderer_Vulkan::create_instance(const char *game_name, const char *engine_name)
    {
        if (m_enable_validation_layers && !validation_layers_are_supported())
            throw std::runtime_error("Validation layers are required but not available.");

        vk::ApplicationInfo app_info(
            game_name, 
            VK_MAKE_API_VERSION(0, 0, 0, 1),
            engine_name,
            VK_MAKE_API_VERSION(0, 0, 0, 1),
            VK_API_VERSION_1_0
        );

        auto extensions = get_required_instance_extensions();
        vk::InstanceCreateInfo create_info({}, 
            &app_info, 
            0, nullptr, 
            static_cast<u32>(extensions.size()), extensions.data(), 
            nullptr
        );

        vk::DebugUtilsMessengerCreateInfoEXT debug_messenger_create_info;
        if (m_enable_validation_layers)
        {
            create_info.enabledLayerCount = static_cast<u32>(m_validation_layers.size());
            create_info.ppEnabledLayerNames = m_validation_layers.data();

            DebugMessenger_Vulkan::populate_debug_messenger_create_info(debug_messenger_create_info);
            create_info.pNext = &debug_messenger_create_info;
        }

        m_instance = vk::createInstance(create_info);
    }

    void Renderer_Vulkan::create_command_pool()
    {
        // TODO: The device wrapper class should return queue families
        auto queue_family_indices = find_queue_families(m_device->physical_device_handle());

        vk::CommandPoolCreateInfo command_pool_info(
            vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
            queue_family_indices.graphics_family.value()
        );

        m_command_pool = m_device->handle().createCommandPool(command_pool_info);
    }

    void Renderer_Vulkan::create_command_buffers()
    {
        m_command_buffers.resize(max_frames_in_flight);

        vk::CommandBufferAllocateInfo alloc_info(
            m_command_pool,
            vk::CommandBufferLevel::ePrimary,
            m_command_buffers.size()
        );

        m_command_buffers = m_device->handle().allocateCommandBuffers(alloc_info);
    }

    void Renderer_Vulkan::create_sync_objects()
    {
        m_image_available_semaphores.resize(max_frames_in_flight);
        m_render_finished_semaphores.resize(max_frames_in_flight);
        m_in_flight_fences.resize(max_frames_in_flight);

        vk::SemaphoreCreateInfo semaphore_info({});
        // Prevent blocking on first draw_frame by creating the fence in the signaled state
        vk::FenceCreateInfo fence_info(vk::FenceCreateFlagBits::eSignaled);

        for (size_t i = 0; i < max_frames_in_flight; i++)
        {
            m_image_available_semaphores[i] = m_device->handle().createSemaphore(semaphore_info);
            m_render_finished_semaphores[i] = m_device->handle().createSemaphore(semaphore_info);
            m_in_flight_fences[i]           = m_device->handle().createFence(fence_info);
        }
    }

    std::vector<const char*> Renderer_Vulkan::get_required_instance_extensions() const
    {
        u32 glfw_extension_count = 0;
        const char** glfw_extensions;
        glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

        std::vector<const char*> extensions(glfw_extensions, glfw_extensions + glfw_extension_count);

        if (m_enable_validation_layers)
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

        return extensions;
    }

    bool Renderer_Vulkan::validation_layers_are_supported() const
    {
        auto available_layers = vk::enumerateInstanceLayerProperties();

        for (const char* layer_name : m_validation_layers)
        {
            bool layer_found = false;

            for (const auto& layer_properties : available_layers)
            {
                if (strcmp(layer_name, layer_properties.layerName) == 0)
                {
                    layer_found = true;
                    break;
                }
            }

            if (!layer_found)
                return false;
        }

        return true;

    }

    Renderer_Vulkan::QueueFamilyIndices Renderer_Vulkan::find_queue_families(
        const vk::PhysicalDevice &device) const
    {
        assert(m_surface);
        
        QueueFamilyIndices indices;

        // TODO: Prioritize queue families with the same queue indices
        auto queue_families = device.getQueueFamilyProperties();

        u32 i = 0;
        for (const auto& queue_family : queue_families)
        {
            if (queue_family.queueFlags & vk::QueueFlagBits::eGraphics)
                indices.graphics_family = i;

            if (m_surface->surface_support(device, i))
                indices.present_family = i;

            if (indices.is_complete())
                break;

            i++;
        }

        return indices;
    }

    vk::SurfaceFormatKHR Renderer_Vulkan::choose_swap_surface_format(const std::vector<vk::SurfaceFormatKHR>& available_formats) const
    {
        for (const auto& format : available_formats)
        {
            if (format.format == vk::Format::eB8G8R8A8Srgb && 
                format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear)
            {
                return format;
            }
        }

        assert(!available_formats.empty());
        return available_formats[0];
    }

    vk::PresentModeKHR Renderer_Vulkan::choose_swap_present_mode(const std::vector<vk::PresentModeKHR>& available_present_modes) const
    {
        // Present modes : 
        // https://vulkan-tutorial.com/en/Drawing_a_triangle/Presentation/swapchain
        
        for (const auto& present_mode : available_present_modes)
        {
            if (present_mode == vk::PresentModeKHR::eMailbox)
                return present_mode;
        }

        return vk::PresentModeKHR::eFifo;
    }

    vk::Extent2D Renderer_Vulkan::choose_swap_extent(
        const IWindow& window, 
        const vk::SurfaceCapabilitiesKHR& capabilities) const
    {
        if (capabilities.currentExtent.width != std::numeric_limits<u32>::max())
            return capabilities.currentExtent;

        auto window_size = window.framebuffer_size();
        vk::Extent2D actual_extent(window_size.x, window_size.y);
        
        actual_extent.width = std::clamp(actual_extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actual_extent.height = std::clamp(actual_extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actual_extent;
    }

    void Renderer_Vulkan::record_command_buffer(const vk::CommandBuffer &command_buffer, u32 image_index) const
    {
        vk::CommandBufferBeginInfo command_buffer_info({}, nullptr);
        command_buffer.begin(command_buffer_info);
            vk::ClearValue clear_value(vk::ClearColorValue(std::array<f32, 4> { 0.0f, 0.0f, 0.0f, 1.0f }));
            vk::RenderPassBeginInfo render_pass_info(
                m_swapchain->render_pass(), 
                m_swapchain->framebuffers()[image_index],
                vk::Rect2D({ 0, 0 }, m_swapchain->extent()),
                1, &clear_value
            );

            command_buffer.beginRenderPass(render_pass_info, vk::SubpassContents::eInline);
                command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipeline->pipeline_handle());
                command_buffer.setViewport(0, m_swapchain->viewport());
                command_buffer.setScissor(0,  m_swapchain->scissor());
                command_buffer.draw(3, 1, 0, 0);
            command_buffer.endRenderPass();
        command_buffer.end();
    }
}