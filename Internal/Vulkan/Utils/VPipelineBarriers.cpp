//
// Created by wpsimon09 on 05/03/25.
//
#include  "VPipelineBarriers.hpp"

#include "Vulkan/VulkanCore/VImage/VImage.hpp"

void VulkanUtils::PlaceImageMemoryBarrier(const VulkanCore::VCommandBuffer& cmdBuffer, const VulkanCore::VImage& image,
    vk::ImageLayout oldLayout, vk::ImageLayout newLayout, vk::PipelineStageFlags srcPipelineStage,
    vk::PipelineStageFlags dstPipelineStage, vk::AccessFlags srcData, vk::AccessFlags dstData)
{
    vk::ImageMemoryBarrier imageMemBarrier{
        srcData,
        dstData,
        oldLayout,
        newLayout,
        vk::QueueFamilyIgnored,
        vk::QueueFamilyIgnored,
        image.GetImage(),
        vk::ImageSubresourceRange{
            vk::ImageAspectFlagBits::eColor,
            0,
            1,
            0,
            1}
    };

    cmdBuffer.GetCommandBuffer().pipelineBarrier(
        srcPipelineStage, dstPipelineStage,
        {},
        0, nullptr,
        0, nullptr,
        1, &imageMemBarrier
    );
}
