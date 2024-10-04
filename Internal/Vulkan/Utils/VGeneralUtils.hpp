//
// Created by wpsimon09 on 27/09/24.
//

#ifndef VGENERALUTILS_HPP
#define VGENERALUTILS_HPP
#include <cstdint>
#include <vulkan/vulkan.hpp>

namespace ApplicationCore
{
    class VertexArray;
}

namespace VulkanUtils
{
    uint32_t FindQueueFamily(const std::vector<vk::QueueFamilyProperties>& queueFamilyProperties, vk::QueueFlagBits queueType);
    vk::ImageView GenerateImageView(const vk::Device& logicalDevice, const vk::Image& image, uint32_t mipLevels = 1, vk::Format format = vk::Format::eR8G8B8A8Srgb, vk::ImageAspectFlags aspecFlags = vk::ImageAspectFlagBits::eColor);

    void TransformVAOToVulkanRepresentation(
        const ApplicationCore::VertexArray& vertexArray, vk::VertexInputBindingDescription &bindingDescription,
        std::vector<vk::VertexInputAttributeDescription> &attributeDescription,
        vk::PrimitiveTopology primitiveTopology = vk::PrimitiveTopology::eTriangleList
        );
}

#endif //VGENERALUTILS_HPP
