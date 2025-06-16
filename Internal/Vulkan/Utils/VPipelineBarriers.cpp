//
// Created by wpsimon09 on 05/03/25.
//
#include "VPipelineBarriers.hpp"

#include "Vulkan/VulkanCore/VImage/VImage.hpp"
#include "Vulkan/VulkanCore/VImage/VImage2.hpp"

void VulkanUtils::PlaceImageMemoryBarrier(VulkanCore::VImage2&        image,
                                          VulkanCore::VCommandBuffer& commandBuffer,
                                          vk::ImageLayout             oldLayout,
                                          vk::ImageLayout             newLayout,
                                          vk::PipelineStageFlags      srcPipelineStage,
                                          vk::PipelineStageFlags      dstPipelineStage,
                                          vk::AccessFlags             srcData,
                                          vk::AccessFlags             dstData)
{
    vk::ImageMemoryBarrier imageMemBarrier{srcData,
                                           dstData,
                                           oldLayout,
                                           newLayout,
                                           vk::QueueFamilyIgnored,
                                           vk::QueueFamilyIgnored,
                                           image.GetImage(),
                                           vk::ImageSubresourceRange{image.GetImageFlags().IsDepthBuffer ?
                                                                         vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil :
                                                                         vk::ImageAspectFlagBits::eColor,
                                                                     0, 1, 0, image.GetImageInfo().arrayLayers}};

    commandBuffer.GetCommandBuffer().pipelineBarrier(srcPipelineStage, dstPipelineStage, {}, 0, nullptr, 0, nullptr, 1, &imageMemBarrier);
}

void VulkanUtils::PlaceImageMemoryBarrier(VulkanCore::VImage2&              image,
                                          const VulkanCore::VCommandBuffer& commandBuffer,
                                          vk::ImageLayout                   oldLayout,
                                          vk::ImageLayout                   newLayout,
                                          vk::PipelineStageFlags            srcPipelineStage,
                                          vk::PipelineStageFlags            dstPipelineStage,
                                          vk::AccessFlags                   srcData,
                                          vk::AccessFlags                   dstData)
{
    vk::ImageMemoryBarrier imageMemBarrier{srcData,
                                           dstData,
                                           oldLayout,
                                           newLayout,
                                           vk::QueueFamilyIgnored,
                                           vk::QueueFamilyIgnored,
                                           image.GetImage(),
                                           vk::ImageSubresourceRange{image.GetImageFlags().IsDepthBuffer ?
                                                                         vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil :
                                                                         vk::ImageAspectFlagBits::eColor,
                                                                     0, 1, 0, image.GetImageInfo().arrayLayers}};

    commandBuffer.GetCommandBuffer().pipelineBarrier(srcPipelineStage, dstPipelineStage, {}, 0, nullptr, 0, nullptr, 1, &imageMemBarrier);
}

void VulkanUtils::PlacePipelineBarrier(const VulkanCore::VCommandBuffer& cmdBuffer, vk::PipelineStageFlags src, vk::PipelineStageFlags dst)
{
    //cmdBuffer.GetCommandBuffer().pipelineBarrier(
    //  src, dst, {},
    //nullptr, nullptr,
    //nullptr, nullptr);
}
void VulkanUtils::PlaceAccelerationStructureMemoryBarrier(const vk::CommandBuffer& cmdBuffer, vk::AccessFlags src, vk::AccessFlags dst)
{
    VkMemoryBarrier barrier{.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER};
    barrier.srcAccessMask = static_cast<VkAccessFlags>(src);
    barrier.dstAccessMask = static_cast<VkAccessFlags>(dst);

    vkCmdPipelineBarrier(cmdBuffer, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
                         VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, 0, 1, &barrier, 0, nullptr, 0, nullptr);
}
void VulkanUtils::PlaceBufferMemoryBarrier(const vk::CommandBuffer& cmdBuffer,
                                           const vk::Buffer&        buffer,
                                           vk::AccessFlags          src,
                                           vk::PipelineStageFlags   piplineSrc,
                                           vk::AccessFlags          dst,
                                           vk::PipelineStageFlags   pipelineDst)
{
    vk::BufferMemoryBarrier barrier {};
    barrier.srcAccessMask = src;
    barrier.dstAccessMask = dst;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.buffer = buffer;
    barrier.offset = {};
    barrier.size = VK_WHOLE_SIZE;


    cmdBuffer.pipelineBarrier(
        piplineSrc,
        pipelineDst,
        {},        // dependencyFlags
        nullptr,   // memoryBarriers
        barrier,   // bufferBarriers
        nullptr    // imageBarriers
    );
}