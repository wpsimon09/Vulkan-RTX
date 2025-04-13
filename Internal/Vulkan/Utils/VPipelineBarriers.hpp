//
// Created by wpsimon09 on 05/03/25.
//

#ifndef VPIPELINEBARRIERS_HPP
#define VPIPELINEBARRIERS_HPP

#include <vulkan/vulkan.hpp>

#include "Vulkan/VulkanCore/Descriptors/VDescriptorSetLayout.hpp"

namespace VulkanCore {
class VImage2;
class VCommandBuffer;
class VImage;
}  // namespace VulkanCore

namespace VulkanUtils {
void PlaceImageMemoryBarrier(VulkanCore::VImage2&        image,
                             VulkanCore::VCommandBuffer& commandBuffer,
                             vk::ImageLayout             oldLayout,
                             vk::ImageLayout             newLayout,
                             vk::PipelineStageFlags      srcPipelineStage,
                             vk::PipelineStageFlags      dstPipelineStage,
                             vk::AccessFlags             srcData,
                             vk::AccessFlags             dstData);

void PlacePipelineBarrier(const VulkanCore::VCommandBuffer& cmdBuffer, vk::PipelineStageFlags src, vk::PipelineStageFlags dst);
}  // namespace VulkanUtils

#endif  //VPIPELINEBARRIERS_HPP
