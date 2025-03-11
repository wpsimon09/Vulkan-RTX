//
// Created by wpsimon09 on 05/03/25.
//

#ifndef VPIPELINEBARRIERS_HPP
#define VPIPELINEBARRIERS_HPP
#include <vulkan/vulkan.hpp>

#include "Vulkan/VulkanCore/Descriptors/VDescriptorSetLayout.hpp"

namespace VulkanCore
{
    class VCommandBuffer;
    class VImage;
}

namespace VulkanUtils
{
    void PlaceImageMemoryBarrier(const VulkanCore::VCommandBuffer& cmdBuffer,const VulkanCore::VImage& image,vk::ImageLayout oldLayout, vk::ImageLayout newLayout,vk::PipelineStageFlags srcPipelineStage, vk::PipelineStageFlags dstPipelineStage,  vk::AccessFlags srcData, vk::AccessFlags dstData);

    void PlacePipelineBarrier(const VulkanCore::VCommandBuffer& cmdBuffer, vk::PipelineStageFlags src,vk::PipelineStageFlags dst);
}

#endif //VPIPELINEBARRIERS_HPP
