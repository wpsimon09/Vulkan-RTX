#pragma once
#include "vulkan/vulkan.hpp"

namespace VulkanCore
{
    class VImage2;
    class VCommandBuffer;
    class VImage;
}


namespace VulkanUtils
{
    void RecordImageTransitionLayoutCommand(
        vk::ImageLayout currentLayout,
        vk::ImageLayout targetLayout,
        vk::ImageMemoryBarrier& barrier,
        VulkanCore::VCommandBuffer& commandBuffer);

    void RecordImageTransitionLayoutCommand(
        const VulkanCore::VImage& image ,
        vk::ImageLayout targetLayout,
        vk::ImageLayout currentLayout,
        VulkanCore::VCommandBuffer& commandBuffer
        );

    void RecordImageTransitionLayoutCommand(
        const VulkanCore::VImage2& image ,
        vk::ImageLayout targetLayout,
        vk::ImageLayout currentLayout,
        VulkanCore::VCommandBuffer& commandBuffer
        );

    std::string ImageLayoutToString(vk::ImageLayout imageLayout);
}
