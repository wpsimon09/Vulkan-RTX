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

/**
 * Places memory barrier specifically tailored towards accelration structure builds
 * @param cmdBuffer command buffer that should contain memory barrier
 * @param src stages to be completed
 * @param dst stages to wait on
 * @todo Redo to the vulkan HPP style instead of the C style, this might require to turn on Snychronisation2 which I was gracefully ignoring :/
 */
void PlaceAccelerationStructureMemoryBarrier(const vk::CommandBuffer& cmdBuffer, vk::AccessFlags src, vk::AccessFlags dst);


}  // namespace VulkanUtils

#endif  //VPIPELINEBARRIERS_HPP
