//
// Created by wpsimon09 on 05/03/25.
//

#ifndef VPIPELINEBARRIERS_HPP
#define VPIPELINEBARRIERS_HPP

#include <vulkan/vulkan.hpp>

namespace VulkanCore {
class VImage2;
class VCommandBuffer;
class VImage;
}  // namespace VulkanCore

namespace VulkanUtils {

struct VBarrierPosition
{
    vk::PipelineStageFlagBits2 srcPipelineStage = vk::PipelineStageFlagBits2::eNone;
    vk::AccessFlags2           srcData          = vk::AccessFlagBits2::eNone;

    vk::PipelineStageFlags2 dstPipelineStage = vk::PipelineStageFlagBits2::eNone;
    vk::AccessFlags2        dstData          = vk::AccessFlagBits2::eNone;
};

void PlaceImageMemoryBarrier(VulkanCore::VImage2&        image,
                             VulkanCore::VCommandBuffer& commandBuffer,
                             vk::ImageLayout             oldLayout,
                             vk::ImageLayout             newLayout,
                             vk::PipelineStageFlags      srcPipelineStage,
                             vk::PipelineStageFlags      dstPipelineStage,
                             vk::AccessFlags             srcData,
                             vk::AccessFlags             dstData);

void PlaceImageMemoryBarrier(VulkanCore::VImage2&              image,
                             const VulkanCore::VCommandBuffer& commandBuffer,
                             vk::ImageLayout                   oldLayout,
                             vk::ImageLayout                   newLayout,
                             vk::PipelineStageFlags            srcPipelineStage,
                             vk::PipelineStageFlags            dstPipelineStage,
                             vk::AccessFlags                   srcData,
                             vk::AccessFlags                   dstData);

void PlaceImageMemoryBarrier2(VulkanCore::VImage2&              image,
                              const VulkanCore::VCommandBuffer& commandBuffer,
                              vk::ImageLayout                   oldLayout,
                              vk::ImageLayout                   newLayout,
                              const VBarrierPosition&           position);

void PlacePipelineBarrier(const VulkanCore::VCommandBuffer& cmdBuffer, vk::PipelineStageFlags src, vk::PipelineStageFlags dst);

/**
 * Places memory barrier specifically tailored towards accelration structure builds
 * @param cmdBuffer command buffer that should contain memory barrier
 * @param src stages to be completed
 * @param dst stages to wait on
 * @todo Redo to the vulkan HPP style instead of the C style, this might require to turn on Snychronisation2 which I was gracefully ignoring :/
 */
void PlaceAccelerationStructureMemoryBarrier(const vk::CommandBuffer& cmdBuffer, vk::AccessFlags src, vk::AccessFlags dst);

void PlaceAccelerationStructureMemoryBarrier2(const vk::CommandBuffer& cmdBuffer, vk::AccessFlags2 src, vk::AccessFlags2 dst);

void PlaceBufferMemoryBarrier(const vk::CommandBuffer& cmdBuffer,
                              const vk::Buffer&        buffer,
                              vk::AccessFlags          src,
                              vk::PipelineStageFlags   piplineSrc,
                              vk::AccessFlags          dst,
                              vk::PipelineStageFlags   pipelineDst);

void PlaceBufferMemoryBarrier2(const vk::CommandBuffer& cmdBuffer, const vk::Buffer& buffer, const VBarrierPosition& position);

void PlaceStallAllBarrier_DEBUG();  // TODO: implement

static constexpr VBarrierPosition VRenderTarget_Color_ToSample_InFragmnetShader_BarrierPosition{
    vk::PipelineStageFlagBits2::eColorAttachmentOutput, vk::AccessFlagBits2::eColorAttachmentWrite,
    vk::PipelineStageFlagBits2::eFragmentShader, vk::AccessFlagBits2::eShaderRead};

static constexpr VBarrierPosition VRenderTarget_Depth_ToSample_InFragmnetShader_BarrierPosition{
    vk::PipelineStageFlagBits2::eColorAttachmentOutput, vk::AccessFlagBits2::eDepthStencilAttachmentWrite,
    vk::PipelineStageFlagBits2::eFragmentShader, vk::AccessFlagBits2::eShaderRead};

}  // namespace VulkanUtils

#endif  //VPIPELINEBARRIERS_HPP
