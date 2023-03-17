#include "renderer_vulkan.h"

#include "core/engine.h"
#include "core/game_info.h"
#include "core/log.h"
#include "core/types.h"
#include "windowing/window.h"
#include "windowing/windowing_module.h"

#include "debug_messenger_vulkan.h"
#include "device_vulkan.h"
#include "pipeline_vulkan.h"
#include "surface_vulkan.h"
#include "swapchain_vulkan.h"

// GLFW for Vulkan
// TODO: Should not assume Vulkan is used with GLFW.
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <iostream>
#undef max
#include <limits>
#include <vulkan/vulkan.h>

namespace engine
{
    Renderer_Vulkan::Renderer_Vulkan()
    {

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

        auto windowing_module = Engine::instance()->get_module<WindowingModule>();

        if (windowing_module)
            windowing_module->window().resize_event().remove_listener(m_window_resize_event_handle);

        m_swapchain.reset();
        m_device->handle().destroy();

        m_surface.reset();
        m_debug_messenger.reset();
        m_instance.destroy();

        vkDestroyInstance(m_instance, nullptr);
    }

    bool Renderer_Vulkan::init(const GameInfo& game_info, IWindow& window)
    {
        if (!create_instance(game_info))
            return false;

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

        m_device = Device_Vulkan::create(shared_from_this());
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

        m_window_resize_event_handle = window.resize_event().add_listener(
            [&](const WindowResizeEvent& event)
            {
                m_swapchain->recreate();
            });

        create_command_pool();

        create_command_buffers();

        create_sync_objects();

        return true;
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

    PipelineCreateData Renderer_Vulkan::create_pipeline()
    {
        return PipelineCreateData(shared_from_this());
    }

    std::shared_ptr<IPipeline> Renderer_Vulkan::compile_pipeline(const PipelineCreateData& create_data)
    {
        return Pipeline_Vulkan::create(shared_from_this(), create_data);
    }

    void Renderer_Vulkan::bind_pipeline(const std::shared_ptr<IPipeline>& pipeline)
    {
        m_pipeline = std::dynamic_pointer_cast<Pipeline_Vulkan>(pipeline);
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

    bool Renderer_Vulkan::create_instance(const GameInfo& game_info)
    {
        if (m_enable_validation_layers && !validation_layers_are_supported())
            return false;

        VkApplicationInfo app_info
        {
            .sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO,
            .pApplicationName   = game_info.game_name.data(),
            .applicationVersion = VK_MAKE_VERSION(0, 0, 1),
            .pEngineName        = game_info.engine_name.data(),
            .engineVersion      = VK_MAKE_VERSION(0, 0, 1),
            .apiVersion         = VK_API_VERSION_1_0
        };

        auto extensions = get_required_instance_extensions();
        VkInstanceCreateInfo create_info
        {
            .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
            .pApplicationInfo = &app_info,
            .enabledLayerCount = 0,
            .ppEnabledLayerNames = nullptr,
            .enabledExtensionCount = static_cast<u32>(extensions.size()),
            .ppEnabledExtensionNames = extensions.data(),
        };

        VkDebugUtilsMessengerCreateInfoEXT messenger_create_info{};
        if (m_enable_validation_layers)
        {
            create_info.enabledLayerCount = static_cast<u32>(m_validation_layers.size());
            create_info.ppEnabledLayerNames = m_validation_layers.data();

            DebugMessenger_Vulkan::populate_debug_messenger_create_info(messenger_create_info);
            create_info.pNext = &messenger_create_info;
        }

        return vkCreateInstance(&create_info, nullptr, &m_instance) == VK_SUCCESS;
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

        vk::SemaphoreCreateInfo semaphore_info;
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
        u32 layer_count;
        if (vkEnumerateInstanceLayerProperties(&layer_count, nullptr) != VK_SUCCESS)
            return false;

        std::vector<VkLayerProperties> available_layers(layer_count);
        if (!vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data()))
            return false;

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
                command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipeline->handle());
                command_buffer.setViewport(0, m_swapchain->viewport());
                command_buffer.setScissor(0,  m_swapchain->scissor());
                command_buffer.draw(3, 1, 0, 0);
            command_buffer.endRenderPass();
        command_buffer.end();
    }
}