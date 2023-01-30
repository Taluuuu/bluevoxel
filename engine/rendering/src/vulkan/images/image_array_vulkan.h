#pragma once

#include <optional>
#include <memory>
#include <vector>
#include <vulkan/vulkan.hpp>

namespace engine
{
    class Image_Vulkan;
    class ImageView_Vulkan;

    class ImageArray_Vulkan
    {
    public:

        static std::optional<ImageArray_Vulkan> create(
            const std::vector<vk::Image>& images_vk, 
            const vk::Format& format, 
            const vk::Device& device,
            bool destroy_handles = true);

        ImageArray_Vulkan(const ImageArray_Vulkan&) = default;
        ImageArray_Vulkan(ImageArray_Vulkan&& other);

        std::vector<ImageView_Vulkan> create_image_views();

    private:

        ImageArray_Vulkan(
            const std::vector<std::shared_ptr<Image_Vulkan>>& images);

    private:

        std::vector<std::shared_ptr<Image_Vulkan>> m_images;

    };
}