#pragma once

#include "image_view_vulkan.h"

#include <memory>
#include <optional>
#include <vector>
#include <vulkan/vulkan.hpp>

namespace engine
{
    class Image_Vulkan : public std::enable_shared_from_this<Image_Vulkan>
    {
    public:

        static std::shared_ptr<Image_Vulkan> create(
            const vk::Image& image, 
            const vk::Format& format, 
            const vk::Device& device,
            bool destroy_handle = true);

        ~Image_Vulkan();
        Image_Vulkan(Image_Vulkan&& other);
        Image_Vulkan(const Image_Vulkan&) = delete;

        const vk::Image&  handle() const { return m_image;  }
        const vk::Format& format() const { return m_format; }
        const vk::Device& device() const { return m_device; }

        std::optional<ImageView_Vulkan> create_image_view() const;

    private:

        Image_Vulkan(
            const vk::Image&  image,
            const vk::Format& format,
            const vk::Device& device,
            bool destroy_handle);
    
    private:

        vk::Image  m_image  = nullptr;
        vk::Format m_format;
        bool m_destroy_handle = true;

        vk::Device m_device = nullptr;

    };
}