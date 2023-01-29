#pragma once

#include <memory>
#include <optional>
#include <vulkan/vulkan.hpp>

namespace engine
{
    class Image_Vulkan;

    class ImageView_Vulkan
    {
    public:

        static std::optional<ImageView_Vulkan> create(
            const std::shared_ptr<const Image_Vulkan>& image);

        ~ImageView_Vulkan();
        ImageView_Vulkan(ImageView_Vulkan&& other);
        ImageView_Vulkan(const ImageView_Vulkan&) = delete;

    private:

        ImageView_Vulkan(
            const vk::ImageView& image_view,
            const std::shared_ptr<const Image_Vulkan>& image);

    private:

        vk::ImageView                       m_image_view = nullptr;

        std::shared_ptr<const Image_Vulkan> m_image      = nullptr;
        vk::Device                          m_device     = nullptr;

    };
}