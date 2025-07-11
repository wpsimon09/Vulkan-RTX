#pragma once
#include "vulkan/vulkan.hpp"

namespace VulkanCore {
class VImage2;
class VCommandBuffer;
class VImage;
}  // namespace VulkanCore


namespace VulkanUtils {
void ApplyTransition(vk::ImageLayout             currentLayout,
                     vk::ImageLayout             targetLayout,
                     vk::ImageMemoryBarrier&     barrier,
                     VulkanCore::VCommandBuffer& commandBuffer);

void ApplyTransition(vk::ImageLayout          currentLayout,
                     vk::ImageLayout          targetLayout,
                     vk::ImageMemoryBarrier&  barrier,
                     const vk::CommandBuffer& commandBuffer);

void RecordImageTransitionLayoutCommand(VulkanCore::VImage2&        image,
                                        vk::ImageLayout             targetLayout,
                                        vk::ImageLayout             currentLayout,
                                        VulkanCore::VCommandBuffer& commandBuffer);

void RecordImageTransitionLayoutCommand(VulkanCore::VImage2&     image,
                                        vk::ImageLayout          targetLayout,
                                        vk::ImageLayout          currentLayout,
                                        const vk::CommandBuffer& commandBuffer);

void EvaluateBarrierMasks(vk::ImageLayout         targetLayout,
                          vk::ImageLayout         currentLayout,
                          vk::ImageMemoryBarrier& barrier,
                          vk::PipelineStageFlags& srcStageFlags,
                          vk::PipelineStageFlags& dstStageFlags);

std::string ImageLayoutToString(vk::ImageLayout imageLayout);
}  // namespace VulkanUtils
