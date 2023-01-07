#include "renderer_vulkan.h"

#include "core/game_info.h"
#include "core/types.h"
#include "core/utils.h"
#include "windowing/window.h"
#include "shader_vulkan.h"

// GLFW for Vulkan
// TODO: this file should not assume Vulkan is used with GLFW.
#define VK_USE_PLATFORM_WIN32_KHR
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <iostream>
#undef max
#include <limits>
#include <set>

PFN_vkCreateDebugUtilsMessengerEXT  pfnVkCreateDebugUtilsMessengerEXT;
PFN_vkDestroyDebugUtilsMessengerEXT pfnVkDestroyDebugUtilsMessengerEXT;

VKAPI_ATTR VkResult VKAPI_CALL vkCreateDebugUtilsMessengerEXT(
    VkInstance instance,
    const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkDebugUtilsMessengerEXT* pMessenger )
{
    return pfnVkCreateDebugUtilsMessengerEXT(instance, pCreateInfo, pAllocator, pMessenger);
}

VKAPI_ATTR void VKAPI_CALL vkDestroyDebugUtilsMessengerEXT(
    VkInstance instance, 
    VkDebugUtilsMessengerEXT messenger, 
    VkAllocationCallbacks const* pAllocator)
{
    return pfnVkDestroyDebugUtilsMessengerEXT(instance, messenger, pAllocator);
}

static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT message_severity,
    VkDebugUtilsMessageTypeFlagsEXT message_type,
    const VkDebugUtilsMessengerCallbackDataEXT* callback_data,
    void* user_data)
{
    std::cerr << "validation layer: " << callback_data->pMessage << std::endl;
    return VK_FALSE;
}

namespace engine
{
    Renderer_Vulkan::Renderer_Vulkan(const GameInfo& game_info, const IWindow& window)
    {
        // TODO: Make sure to destroy the instance if setup_debug_messenger fails.
        create_instance(game_info.game_name.data(), game_info.engine_name.data());
        setup_debug_messenger();
        create_surface(window);
        pick_physical_device();
        create_logical_device();
        create_swapchain(window);
        create_image_views();
        create_render_pass();
        create_graphics_pipeline();
        create_framebuffers();
        create_command_pool();
        create_command_buffer();
        create_sync_objects();
    }

    Renderer_Vulkan::~Renderer_Vulkan()
    {
        m_device.destroySemaphore(m_image_available_semaphore);
        m_device.destroySemaphore(m_render_finished_semaphore);
        m_device.destroyFence(m_in_flight_fence);

        m_device.destroyCommandPool(m_command_pool);

        for (const auto& framebuffer : m_swapchain_framebuffers)
            m_device.destroyFramebuffer(framebuffer);

        m_device.destroyPipeline(m_graphics_pipeline);
        m_device.destroyPipelineLayout(m_pipeline_layout);
        m_device.destroyRenderPass(m_render_pass);

        for (const auto& image_view : m_swapchain_image_views)
            m_device.destroyImageView(image_view);

        m_device.destroySwapchainKHR(m_swapchain);
        m_device.destroy();

        m_instance.destroySurfaceKHR(m_surface);
        m_instance.destroyDebugUtilsMessengerEXT(m_debug_messenger);
        m_instance.destroy();
    }

    void Renderer_Vulkan::draw_frame() const
    {
        // TODO: Figure out why nodiscard
        auto wait_fence_result = m_device.waitForFences(m_in_flight_fence, VK_TRUE, UINT64_MAX);
        m_device.resetFences(m_in_flight_fence);

        // TODO: Check this .value i'm really fucking tired so i trust future me to do it
        u32 image_index = 
            m_device.acquireNextImageKHR(m_swapchain, UINT64_MAX, m_image_available_semaphore, nullptr).value;

        m_command_buffer.reset();
        record_command_buffer(m_command_buffer, image_index);

        vk::Semaphore wait_semaphores[]   = { m_image_available_semaphore };
        vk::Semaphore signal_semaphores[] = { m_render_finished_semaphore };
        vk::PipelineStageFlags wait_stages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
        vk::SubmitInfo submit_info(
            1, wait_semaphores,
            wait_stages,
            1, &m_command_buffer,
            1, signal_semaphores
        );

        m_graphics_queue.submit(submit_info, m_in_flight_fence);

        vk::PresentInfoKHR present_info(
            1, signal_semaphores,
            1, &m_swapchain,
            &image_index,
            nullptr
        );

        // TODO: Figure out why nodiscard
        auto present_result = m_present_queue.presentKHR(present_info);

        // This may be better elsewhere
        m_device.waitIdle();
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

            populate_debug_messenger_create_info(debug_messenger_create_info);
            create_info.pNext = &debug_messenger_create_info;
        }

        m_instance = vk::createInstance(create_info);
    }

    void Renderer_Vulkan::setup_debug_messenger()
    {
        if (!m_enable_validation_layers)
            return;

        pfnVkCreateDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(m_instance.getProcAddr("vkCreateDebugUtilsMessengerEXT"));
        if (!pfnVkCreateDebugUtilsMessengerEXT)
            throw std::runtime_error("GetInstanceProcAddr: Unable to find pfnVkCreateDebugUtilsMessengerEXT function.");

        pfnVkDestroyDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(m_instance.getProcAddr( "vkDestroyDebugUtilsMessengerEXT"));
        if (!pfnVkDestroyDebugUtilsMessengerEXT)
            throw std::runtime_error("GetInstanceProcAddr: Unable to find pfnVkDestroyDebugUtilsMessengerEXT function.");

        vk::DebugUtilsMessengerCreateInfoEXT create_info;
        populate_debug_messenger_create_info(create_info);

        m_debug_messenger = m_instance.createDebugUtilsMessengerEXT(create_info);
    }

    void Renderer_Vulkan::create_surface(const IWindow& window)
    {
        VkSurfaceKHR surface = m_surface;
        auto result = glfwCreateWindowSurface(
            m_instance, 
            reinterpret_cast<GLFWwindow*>(window.wrapped_window_handle()), 
            nullptr, 
            &surface);

        m_surface = surface;

        if (result != VK_SUCCESS)
            throw std::runtime_error("Failed to create GLFW window surface.");
    }

    void Renderer_Vulkan::pick_physical_device()
    {
        std::vector<vk::PhysicalDevice> devices = m_instance.enumeratePhysicalDevices();
        if (devices.empty())
            throw std::runtime_error("Failed to find GPUs with Vulkan support.");

        for (const auto& device : devices)
        {
            if (is_device_suitable(device))
            {
                m_physical_device = device;
                break;
            }
        }

        if (!m_physical_device)
            throw std::runtime_error("Failed to find GPUs with Vulkan support.");
    }

    void Renderer_Vulkan::create_logical_device()
    {
        QueueFamilyIndices indices = find_queue_families(m_physical_device);

        // TODO: .value() could throw - there might be more instances of this in this file.
        std::vector<vk::DeviceQueueCreateInfo> queue_create_infos;
        std::set<uint32_t> unique_queue_families = 
        {
            indices.graphics_family.value(),
            indices.present_family.value()
        };

        f32 queue_priority = 1.0f;
        for (u32 queue_family : unique_queue_families)
        {
            vk::DeviceQueueCreateInfo queue_create_info({}, 
                queue_family, 1, &queue_priority);

            queue_create_infos.push_back(queue_create_info);
        }
        
        vk::PhysicalDeviceFeatures device_features;

        vk::DeviceCreateInfo create_info({}, 
            static_cast<u32>(queue_create_infos.size()),  queue_create_infos.data(),  // Queue create infos
            0,                                            nullptr,                    // Validation layers
            static_cast<u32>(m_device_extensions.size()), m_device_extensions.data(), // Device extensions
            &device_features, 
            nullptr);

        if (m_enable_validation_layers)
        {
            create_info.enabledLayerCount = static_cast<u32>(m_validation_layers.size());
            create_info.ppEnabledLayerNames = m_validation_layers.data();
        }

        m_device         = m_physical_device.createDevice(create_info);
        m_graphics_queue = m_device.getQueue(indices.graphics_family.value(), 0);
        m_present_queue  = m_device.getQueue(indices.present_family.value(), 0);
    }

    void Renderer_Vulkan::create_swapchain(const IWindow& window)
    {
        auto swapchain_support = query_swapchain_support(m_physical_device);
        
        auto surface_format = choose_swap_surface_format(swapchain_support.formats);
        auto present_mode   = choose_swap_present_mode(swapchain_support.present_modes);
        auto extent         = choose_swap_extent(window, swapchain_support.capabilities);

        u32 image_count = swapchain_support.capabilities.minImageCount + 1;
        if (swapchain_support.capabilities.maxImageCount > 0 && 
            image_count > swapchain_support.capabilities.maxImageCount)
        {
            image_count = swapchain_support.capabilities.maxImageCount;
        }

        // The vk::ImageUsageFlagBits parameter would be different if I need to draw
        // to a separate image first to do post processing
        // https://vulkan-tutorial.com/en/Drawing_a_triangle/Presentation/swapchain
        vk::SwapchainCreateInfoKHR create_info({},
            m_surface, 
            image_count, 
            surface_format.format, 
            surface_format.colorSpace, 
            extent, 
            1, 
            vk::ImageUsageFlagBits::eColorAttachment, 
            vk::SharingMode::eExclusive,
            0, nullptr, 
            swapchain_support.capabilities.currentTransform,
            vk::CompositeAlphaFlagBitsKHR::eOpaque,
            present_mode,
            true,
            nullptr
        );

        auto indices = find_queue_families(m_physical_device);
        u32 queue_family_indices[] = 
        {
            indices.graphics_family.value(), 
            indices.present_family.value()
        };

        // Exclusive sharing mode offers best performance
        if (indices.graphics_family != indices.present_family)
        {
            create_info.imageSharingMode = vk::SharingMode::eConcurrent;
            create_info.queueFamilyIndexCount = 2;
            create_info.pQueueFamilyIndices = queue_family_indices;
        }

        m_swapchain              = m_device.createSwapchainKHR(create_info);
        m_swapchain_images       = m_device.getSwapchainImagesKHR(m_swapchain);
        m_swapchain_image_format = surface_format.format;
        m_swapchain_extent       = extent;
    }

    void Renderer_Vulkan::create_image_views()
    {
        m_swapchain_image_views.resize(m_swapchain_images.size());

        vk::ComponentMapping components(
            vk::ComponentSwizzle::eIdentity,
            vk::ComponentSwizzle::eIdentity,
            vk::ComponentSwizzle::eIdentity,
            vk::ComponentSwizzle::eIdentity
        );
        
        vk::ImageSubresourceRange subresource_range(
            vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1);

        for (size_t i = 0; i < m_swapchain_images.size(); i++)
        {
            vk::ImageViewCreateInfo create_info({},
                m_swapchain_images[i],
                vk::ImageViewType::e2D,
                m_swapchain_image_format,
                components,
                subresource_range
            );

            m_swapchain_image_views[i] = m_device.createImageView(create_info);
        }
    }

    void Renderer_Vulkan::create_render_pass()
    {
        vk::AttachmentDescription color_attachment({},
            m_swapchain_image_format,
            vk::SampleCountFlagBits::e1, // Should match the format of the swapchain images
            vk::AttachmentLoadOp::eClear, // Clear screen to black
            vk::AttachmentStoreOp::eStore, // Store rendered content
            vk::AttachmentLoadOp::eDontCare,
            vk::AttachmentStoreOp::eDontCare,
            vk::ImageLayout::eUndefined,
            vk::ImageLayout::ePresentSrcKHR
        );

        vk::AttachmentReference color_attachment_ref(
            0, vk::ImageLayout::eColorAttachmentOptimal
        );

        vk::SubpassDescription subpass({},
            vk::PipelineBindPoint::eGraphics,
            0, nullptr,
            // The index of the attachment in this array is directly referenced from the fragment 
            // shader with the layout(location = 0)
            1, &color_attachment_ref, 
            nullptr,
            nullptr,
            0, nullptr
        );

        vk::SubpassDependency dependency(
            VK_SUBPASS_EXTERNAL, 0,
            vk::PipelineStageFlagBits::eColorAttachmentOutput, 
            vk::PipelineStageFlagBits::eColorAttachmentOutput,
            {},
            vk::AccessFlagBits::eColorAttachmentWrite
        );

        vk::RenderPassCreateInfo render_pass_create_info({},
            1, &color_attachment,
            1, &subpass,
            1, &dependency
        );

        m_render_pass = m_device.createRenderPass(render_pass_create_info);
    }

    void Renderer_Vulkan::create_graphics_pipeline()
    {
        auto vert_shader_code = utils::read_file("Resources/engine/shaders/triangle.vert.spv");
        if (!vert_shader_code.has_value())
            throw std::runtime_error("Failed to read vertex shader.");

        auto frag_shader_code = utils::read_file("Resources/engine/shaders/triangle.frag.spv");
        if (!frag_shader_code.has_value())
            throw std::runtime_error("Failed to read fragment shader.");

        auto vert_shader_module = create_shader_module(*vert_shader_code);
        auto frag_shader_module = create_shader_module(*frag_shader_code);

        vk::PipelineShaderStageCreateInfo vert_shader_stage_create_info({},
            vk::ShaderStageFlagBits::eVertex,
            vert_shader_module,
            "main",
            nullptr // SpecializationInfo, used to set constants at runtime
        );

        vk::PipelineShaderStageCreateInfo frag_shader_stage_create_info({},
            vk::ShaderStageFlagBits::eFragment,
            frag_shader_module,
            "main",
            nullptr // SpecializationInfo, used to set constants at runtime
        );

        vk::PipelineShaderStageCreateInfo shader_stages_create_infos[] = 
        {
            vert_shader_stage_create_info, 
            frag_shader_stage_create_info
        };

        // Vertex input format
        // Bindings   : Spacing between data and whether the data is per-vertex or 
        //              per-instance
        // Attributes : Type of the attributes passed to the vertex shader, which 
        //              binding to load them from and at which offset
        vk::PipelineVertexInputStateCreateInfo vertex_input_info({},
            0, nullptr, // Vertex binding descriptions
            0, nullptr  // Vertex attribute descriptions
        );

        vk::PipelineInputAssemblyStateCreateInfo assembly_info({},
            vk::PrimitiveTopology::eTriangleList,
            false
        );

        vk::Viewport viewport(
            0.0f, 0.0f,
            static_cast<f32>(m_swapchain_extent.width), static_cast<f32>(m_swapchain_extent.height),
            0.0f, 1.0f
        );

        vk::Rect2D scissor(
            { 0, 0 },
            m_swapchain_extent
        );

        std::vector<vk::DynamicState> dynamic_states = 
        {
            vk::DynamicState::eViewport,
            vk::DynamicState::eScissor
        };
        vk::PipelineDynamicStateCreateInfo pipeline_dynamic_state_create_info({},
            static_cast<u32>(dynamic_states.size()),
            dynamic_states.data()
        );

        vk::PipelineViewportStateCreateInfo viewport_state_create_info({},
            1, &viewport, 
            1, &scissor
        );

        vk::PipelineRasterizationStateCreateInfo pipeline_rasterization_state_create_info({},
            VK_FALSE, // Might be useful for shadow mapping
            VK_FALSE,
            vk::PolygonMode::eFill,
            vk::CullModeFlagBits::eBack,
            vk::FrontFace::eClockwise,
            VK_FALSE, 0.0f, 0.0f, 0.0f,
            1.0f
        );

        // Disabled multisampling
        vk::PipelineMultisampleStateCreateInfo pipeline_multisample_state_create_info({},
            vk::SampleCountFlagBits::e1,
            VK_FALSE,
            1.0f, nullptr, VK_FALSE, VK_FALSE // This line is optional as multisampling is disabled
        );

        // Search alpha blending for transparency
        vk::PipelineColorBlendAttachmentState pipeline_color_blend_attachment_state(
            VK_FALSE,
            // The following is optional as blending is disabled
            // Color blend
            vk::BlendFactor::eOne,
            vk::BlendFactor::eZero,
            vk::BlendOp::eAdd,
            // Alpha blend
            vk::BlendFactor::eOne,
            vk::BlendFactor::eZero,
            vk::BlendOp::eAdd,
            // Not optional
            vk::ColorComponentFlagBits::eR |
            vk::ColorComponentFlagBits::eG |
            vk::ColorComponentFlagBits::eB |
            vk::ColorComponentFlagBits::eA
        );

        vk::PipelineColorBlendStateCreateInfo pipeline_color_blend_state_create_info({},
            VK_FALSE,
            vk::LogicOp::eCopy,
            1, &pipeline_color_blend_attachment_state,
            { 0.0f, 0.0f, 0.0f, 0.0f }
        );

        // Will be used to set uniforms
        vk::PipelineLayoutCreateInfo pipeline_layout_create_info({},
            0, nullptr,
            0, nullptr
        );
        m_pipeline_layout = m_device.createPipelineLayout(pipeline_layout_create_info);

        // TODO: Rework naming for create infos, currently a mess
        vk::GraphicsPipelineCreateInfo graphics_pipeline_create_info({},
            2, shader_stages_create_infos,
            &vertex_input_info,
            &assembly_info,
            nullptr,
            &viewport_state_create_info,
            &pipeline_rasterization_state_create_info,
            &pipeline_multisample_state_create_info,
            nullptr,
            &pipeline_color_blend_state_create_info,
            &pipeline_dynamic_state_create_info,
            m_pipeline_layout,
            m_render_pass,
            0,
            nullptr,
            -1
        );

        auto pipeline_creation_result = m_device.createGraphicsPipeline(nullptr, graphics_pipeline_create_info);
        if (pipeline_creation_result.result != vk::Result::eSuccess)
            throw std::runtime_error("Failed to create graphics pipeline.");

        m_graphics_pipeline = pipeline_creation_result.value;

        m_device.destroyShaderModule(vert_shader_module);
        m_device.destroyShaderModule(frag_shader_module);
    }

    void Renderer_Vulkan::create_framebuffers()
    {
        m_swapchain_framebuffers.resize(m_swapchain_image_views.size());

        // Create framebuffers from swapchain image views
        for (size_t i = 0; i < m_swapchain_image_views.size(); i++)
        {
            vk::ImageView attachments[] =
            {
                m_swapchain_image_views[i]
            };

            vk::FramebufferCreateInfo framebuffer_info({},
                m_render_pass,
                1, attachments,
                m_swapchain_extent.width, m_swapchain_extent.height, 1
            );

            m_swapchain_framebuffers[i] = m_device.createFramebuffer(framebuffer_info);
        }
    }

    void Renderer_Vulkan::create_command_pool()
    {
        auto queue_family_indices = find_queue_families(m_physical_device);

        vk::CommandPoolCreateInfo command_pool_info(
            vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
            queue_family_indices.graphics_family.value()
        );

        m_command_pool = m_device.createCommandPool(command_pool_info);
    }

    void Renderer_Vulkan::create_command_buffer()
    {
        vk::CommandBufferAllocateInfo alloc_info(
            m_command_pool,
            vk::CommandBufferLevel::ePrimary,
            1
        );

        // TODO: Probably unsafe
        m_command_buffer = m_device.allocateCommandBuffers(alloc_info)[0];
    }

    void Renderer_Vulkan::create_sync_objects()
    {
        vk::SemaphoreCreateInfo semaphore_info({});
        // Prevent blocking on first draw_frame by creating the fence in the signaled state
        vk::FenceCreateInfo fence_info(vk::FenceCreateFlagBits::eSignaled);

        m_image_available_semaphore = m_device.createSemaphore(semaphore_info);
        m_render_finished_semaphore = m_device.createSemaphore(semaphore_info);
        m_in_flight_fence           = m_device.createFence(fence_info);
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

    void Renderer_Vulkan::populate_debug_messenger_create_info(vk::DebugUtilsMessengerCreateInfoEXT& debug_utils_messenger_create_info) const
    {
        const vk::DebugUtilsMessageSeverityFlagsEXT message_severity_flags = 
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | 
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning | 
            vk::DebugUtilsMessageSeverityFlagBitsEXT::eError;

        const vk::DebugUtilsMessageTypeFlagsEXT message_type_flags = 
            vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
            vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
            vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation;

        debug_utils_messenger_create_info = vk::DebugUtilsMessengerCreateInfoEXT({},
            message_severity_flags,
            message_type_flags,
            debug_callback
        );
    }
    
    bool Renderer_Vulkan::is_device_suitable(const vk::PhysicalDevice& device) const
    {
        // https://vulkan-tutorial.com/Drawing_a_triangle/Setup/Physical_devices_and_queue_families
        // We can evaluate with more criterias which devices we want

        bool extensions_supported = device_supports_extensions(device);

        bool is_swapchain_adequate = false;
        if (extensions_supported)
        {
            auto swapchain_support = query_swapchain_support(device);
            is_swapchain_adequate = swapchain_support.is_adequate();
        }

        QueueFamilyIndices indices = find_queue_families(device);
        return indices.is_complete() && extensions_supported && is_swapchain_adequate;
    }

    bool Renderer_Vulkan::device_supports_extensions(const vk::PhysicalDevice &device) const
    {
        auto available_extensions = device.enumerateDeviceExtensionProperties();
        std::set<std::string_view> required_extensions(
            m_device_extensions.begin(), m_device_extensions.end());

        for (const auto& extension : available_extensions)
            required_extensions.erase(extension.extensionName);

        return required_extensions.empty();
    }

    vk::ShaderModule Renderer_Vulkan::create_shader_module(const std::vector<char> &code) const
    {
        vk::ShaderModuleCreateInfo create_info({}, 
            code.size(),
            reinterpret_cast<const u32*>(code.data())
        );

        return m_device.createShaderModule(create_info);
    }

    Renderer_Vulkan::QueueFamilyIndices Renderer_Vulkan::find_queue_families(
        const vk::PhysicalDevice& device) const
    {
        QueueFamilyIndices indices;

        // TODO: Prioritize queue families with the same queue indices
        auto queue_families = device.getQueueFamilyProperties();

        u32 i = 0;
        for (const auto& queue_family : queue_families)
        {
            if (queue_family.queueFlags & vk::QueueFlagBits::eGraphics)
                indices.graphics_family = i;

            if (device.getSurfaceSupportKHR(i, m_surface))
                indices.present_family = i;

            if (indices.is_complete())
                break;

            i++;
        }

        return indices;
    }

    Renderer_Vulkan::SwapChainSupportDetails Renderer_Vulkan::query_swapchain_support(
        const vk::PhysicalDevice& device) const
    {
        return SwapChainSupportDetails
        {
            .capabilities  = device.getSurfaceCapabilitiesKHR(m_surface),
            .formats       = device.getSurfaceFormatsKHR(m_surface),
            .present_modes = device.getSurfacePresentModesKHR(m_surface),
        };
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
                m_render_pass, 
                m_swapchain_framebuffers[image_index],
                vk::Rect2D({ 0, 0 }, m_swapchain_extent),
                1, &clear_value
            );

            command_buffer.beginRenderPass(render_pass_info, vk::SubpassContents::eInline);
                command_buffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_graphics_pipeline);

                vk::Viewport viewport(
                    0.0f, 0.0f,
                    static_cast<f32>(m_swapchain_extent.width), static_cast<f32>(m_swapchain_extent.height),
                    0.0f, 1.0f
                );
                command_buffer.setViewport(0, viewport);

                vk::Rect2D scissor(
                    { 0, 0 },
                    m_swapchain_extent
                );
                command_buffer.setScissor(0, scissor);

                command_buffer.draw(3, 1, 0, 0);
            command_buffer.endRenderPass();
        command_buffer.end();
    }
}