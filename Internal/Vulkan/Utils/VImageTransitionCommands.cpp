//
// Created by wpsimon09 on 06/03/25.
//

#include "VIimageTransitionCommands.hpp"
#include "Vulkan/VulkanCore/CommandBuffer/VCommandBuffer.hpp"
#include "Vulkan/VulkanCore/VImage/VImage.hpp"
#include "Vulkan/VulkanCore/VImage/VImage2.hpp"

void VulkanUtils::ApplyTransition(vk::ImageLayout             currentLayout,
                                  vk::ImageLayout             targetLayout,
                                  vk::ImageMemoryBarrier&     barrier,
                                  VulkanCore::VCommandBuffer& commandBuffer)
{
    vk::PipelineStageFlags srcStageFlags;
    vk::PipelineStageFlags dstStageFlags;
    EvaluateBarrierMasks(targetLayout, currentLayout, barrier, srcStageFlags, dstStageFlags);
    assert(commandBuffer.GetIsRecording()
           && "Command buffer is not recording the commands enable it by calling commandBuffer->StartRecording()");
    commandBuffer.GetCommandBuffer().pipelineBarrier(srcStageFlags, dstStageFlags, {}, 0, nullptr, 0, nullptr, 1, &barrier);
}
void VulkanUtils::ApplyTransition(vk::ImageLayout          currentLayout,
                                  vk::ImageLayout          targetLayout,
                                  vk::ImageMemoryBarrier&  barrier,
                                  const vk::CommandBuffer& commandBuffer)
{
    vk::PipelineStageFlags srcStageFlags;
    vk::PipelineStageFlags dstStageFlags;
    EvaluateBarrierMasks(targetLayout, currentLayout, barrier, srcStageFlags, dstStageFlags);
    commandBuffer.pipelineBarrier(srcStageFlags, dstStageFlags, {}, 0, nullptr, 0, nullptr, 1, &barrier);
}

void VulkanUtils::RecordImageTransitionLayoutCommand(VulkanCore::VImage2&        image,
                                                     vk::ImageLayout             targetLayout,
                                                     vk::ImageLayout             currentLayout,
                                                     VulkanCore::VCommandBuffer& commandBuffer)
{
    vk::PipelineStageFlags srcStageFlags;
    vk::PipelineStageFlags dstStageFlags;

    vk::ImageMemoryBarrier barrier{};
    barrier.oldLayout           = currentLayout;
    barrier.newLayout           = targetLayout;
    barrier.srcQueueFamilyIndex = vk::QueueFamilyIgnored;
    barrier.dstQueueFamilyIndex = vk::QueueFamilyIgnored;
    barrier.image               = image.GetImage();
    ;
    barrier.subresourceRange.aspectMask     = image.GetImageFlags().IsDepthBuffer ?
                                                  vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil :
                                                  vk::ImageAspectFlagBits::eColor;
    barrier.subresourceRange.baseMipLevel   = 0;
    barrier.subresourceRange.levelCount     = image.GetImageInfo().mipLevels;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount     = image.GetImageInfo().arrayLayers;

    image.GetImageInfo().layout = targetLayout;

    VulkanUtils::ApplyTransition(currentLayout, targetLayout, barrier, commandBuffer);
}
void VulkanUtils::RecordImageTransitionLayoutCommand(VulkanCore::VImage2& image,
                                                     vk::ImageLayout      targetLayout,
                                                     vk::ImageLayout      currentLayout,
                                                     const vk::CommandBuffer&   commandBuffer)
{
    vk::PipelineStageFlags srcStageFlags;
    vk::PipelineStageFlags dstStageFlags;

    vk::ImageMemoryBarrier barrier{};
    barrier.oldLayout           = currentLayout;
    barrier.newLayout           = targetLayout;
    barrier.srcQueueFamilyIndex = vk::QueueFamilyIgnored;
    barrier.dstQueueFamilyIndex = vk::QueueFamilyIgnored;
    barrier.image               = image.GetImage();
    ;
    barrier.subresourceRange.aspectMask     = image.GetImageFlags().IsDepthBuffer ?
                                                  vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil :
                                                  vk::ImageAspectFlagBits::eColor;
    barrier.subresourceRange.baseMipLevel   = 0;
    barrier.subresourceRange.levelCount     = image.GetImageInfo().mipLevels;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount     = image.GetImageInfo().arrayLayers;

    image.GetImageInfo().layout = targetLayout;

    VulkanUtils::ApplyTransition(currentLayout, targetLayout, barrier, commandBuffer);
}

void VulkanUtils::EvaluateBarrierMasks(vk::ImageLayout targetLayout, vk::ImageLayout currentLayout,vk::ImageMemoryBarrier& barrier, vk::PipelineStageFlags& srcStageFlags,
    vk::PipelineStageFlags& dstStageFlags) {


    if(currentLayout == vk::ImageLayout::eUndefined && targetLayout == vk::ImageLayout::eTransferDstOptimal)
    {
        barrier.srcAccessMask = {};
        barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

        srcStageFlags = vk::PipelineStageFlagBits::eTopOfPipe;
        dstStageFlags = vk::PipelineStageFlagBits::eTransfer;
    }
    else if(currentLayout == vk::ImageLayout::eTransferDstOptimal && targetLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
    {
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
        barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

        srcStageFlags = vk::PipelineStageFlagBits::eTransfer;
        dstStageFlags = vk::PipelineStageFlagBits::eFragmentShader;
    }
    else if(currentLayout == vk::ImageLayout::eUndefined && targetLayout == vk::ImageLayout::eColorAttachmentOptimal)
    {
        barrier.srcAccessMask = {};
        barrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite | vk::AccessFlagBits::eColorAttachmentRead;

        srcStageFlags = vk::PipelineStageFlagBits::eTopOfPipe;
        dstStageFlags = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    }
    else if(currentLayout == vk::ImageLayout::eUndefined && targetLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal)
    {
        barrier.srcAccessMask = {};
        barrier.dstAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentWrite | vk::AccessFlagBits::eDepthStencilAttachmentRead;

        srcStageFlags = vk::PipelineStageFlagBits::eTopOfPipe;
        dstStageFlags = vk::PipelineStageFlagBits::eEarlyFragmentTests;
    }
    else if(currentLayout == vk::ImageLayout::eColorAttachmentOptimal && targetLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
    {
        barrier.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
        barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

        srcStageFlags = vk::PipelineStageFlagBits::eColorAttachmentOutput;
        dstStageFlags = vk::PipelineStageFlagBits::eFragmentShader;
    }
    else if(currentLayout == vk::ImageLayout::eShaderReadOnlyOptimal && targetLayout == vk::ImageLayout::eTransferSrcOptimal)
    {
        barrier.srcAccessMask = vk::AccessFlagBits::eShaderRead;
        barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;

        srcStageFlags = vk::PipelineStageFlagBits::eFragmentShader;
        dstStageFlags = vk::PipelineStageFlagBits::eTransfer;
    }
    else if(currentLayout == vk::ImageLayout::eTransferSrcOptimal && targetLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
    {
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
        barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

        srcStageFlags = vk::PipelineStageFlagBits::eTransfer;
        dstStageFlags = vk::PipelineStageFlagBits::eFragmentShader;
    }
    else if(currentLayout == vk::ImageLayout::eShaderReadOnlyOptimal && targetLayout == vk::ImageLayout::eColorAttachmentOptimal)
    {
        barrier.srcAccessMask = vk::AccessFlagBits::eShaderRead;
        barrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;

        srcStageFlags = vk::PipelineStageFlagBits::eFragmentShader;
        dstStageFlags = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    }
    else if(currentLayout == vk::ImageLayout::eColorAttachmentOptimal && targetLayout == vk::ImageLayout::ePresentSrcKHR)
    {
        barrier.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
        barrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentRead;

        srcStageFlags = vk::PipelineStageFlagBits::eColorAttachmentOutput;
        dstStageFlags = vk::PipelineStageFlagBits::eColorAttachmentOutput ;
    }
    else if(currentLayout == vk::ImageLayout::eUndefined && targetLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
    {
        barrier.srcAccessMask = {};
        barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

        srcStageFlags = vk::PipelineStageFlagBits::eTopOfPipe;
        dstStageFlags = vk::PipelineStageFlagBits::eFragmentShader;
    }
    else if(currentLayout == vk::ImageLayout::eColorAttachmentOptimal && targetLayout == vk::ImageLayout::eTransferSrcOptimal)
    {
        barrier.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
        barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;

        srcStageFlags = vk::PipelineStageFlagBits::eColorAttachmentOutput;
        dstStageFlags = vk::PipelineStageFlagBits::eTransfer;
    }
    else if(currentLayout == vk::ImageLayout::eColorAttachmentOptimal && targetLayout == vk::ImageLayout::eTransferSrcOptimal)
    {
        barrier.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
        barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;

        srcStageFlags = vk::PipelineStageFlagBits::eColorAttachmentOutput;
        dstStageFlags = vk::PipelineStageFlagBits::eTransfer;
    }
    else if(currentLayout == vk::ImageLayout::eTransferSrcOptimal && targetLayout == vk::ImageLayout::eColorAttachmentOptimal)
    {
        barrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
        barrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;

        srcStageFlags = vk::PipelineStageFlagBits::eTransfer;
        dstStageFlags = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    }
    else if(currentLayout == vk::ImageLayout::ePresentSrcKHR && targetLayout == vk::ImageLayout::eColorAttachmentOptimal)
    {
        barrier.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
        barrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;

        srcStageFlags = vk::PipelineStageFlagBits::eColorAttachmentOutput;
        dstStageFlags = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    }
    else if (currentLayout == vk::ImageLayout::eUndefined && targetLayout == vk::ImageLayout::eGeneral) {
        barrier.srcAccessMask = {};
        barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead | vk::AccessFlagBits::eShaderWrite;

        srcStageFlags = vk::PipelineStageFlagBits::eTopOfPipe;
        dstStageFlags = vk::PipelineStageFlagBits::eRayTracingShaderKHR | vk::PipelineStageFlagBits::eFragmentShader;
    }
    else if (currentLayout == vk::ImageLayout::eShaderReadOnlyOptimal && targetLayout == vk::ImageLayout::eGeneral) {
        barrier.srcAccessMask = vk::AccessFlagBits::eShaderRead;
        barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead | vk::AccessFlagBits::eShaderWrite;

        srcStageFlags = vk::PipelineStageFlagBits::eFragmentShader | vk::PipelineStageFlagBits::eRayTracingShaderKHR;
        dstStageFlags = vk::PipelineStageFlagBits::eRayTracingShaderKHR | vk::PipelineStageFlagBits::eComputeShader  ;
    }
    else if (currentLayout == vk::ImageLayout::eGeneral && targetLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
        barrier.srcAccessMask = vk::AccessFlagBits::eShaderWrite;
        barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

        srcStageFlags = vk::PipelineStageFlagBits::eRayTracingShaderKHR | vk::PipelineStageFlagBits::eComputeShader;
        dstStageFlags = vk::PipelineStageFlagBits::eFragmentShader | vk::PipelineStageFlagBits::eRayTracingShaderKHR;
    }
    else if (currentLayout == vk::ImageLayout::eUndefined && targetLayout == vk::ImageLayout::ePresentSrcKHR) {
        barrier.srcAccessMask = {};
        barrier.dstAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;

        srcStageFlags = vk::PipelineStageFlagBits::eTopOfPipe;
        dstStageFlags = vk::PipelineStageFlagBits::eColorAttachmentOutput; // or eFragmentShader depending on usage
    }
    else if (currentLayout == vk::ImageLayout::eDepthStencilReadOnlyOptimal && targetLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal) {
        barrier.srcAccessMask = vk::AccessFlagBits::eShaderRead;
        barrier.dstAccessMask =
            vk::AccessFlagBits::eDepthStencilAttachmentWrite |
            vk::AccessFlagBits::eColorAttachmentWrite;

        srcStageFlags = vk::PipelineStageFlagBits::eFragmentShader;
        dstStageFlags =
            vk::PipelineStageFlagBits::eLateFragmentTests |
            vk::PipelineStageFlagBits::eColorAttachmentOutput;

    }
    else if (currentLayout == vk::ImageLayout::eUndefined && targetLayout == vk::ImageLayout::eDepthStencilReadOnlyOptimal) {
        barrier.srcAccessMask = vk::AccessFlagBits::eNone;
        barrier.dstAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentRead ;

        srcStageFlags = vk::PipelineStageFlagBits::eTopOfPipe;
        dstStageFlags = vk::PipelineStageFlagBits::eEarlyFragmentTests;

    }
    else if (currentLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal && targetLayout == vk::ImageLayout::eDepthStencilReadOnlyOptimal) {
        barrier.srcAccessMask =
        vk::AccessFlagBits::eDepthStencilAttachmentRead |
        vk::AccessFlagBits::eDepthStencilAttachmentWrite |
        vk::AccessFlagBits::eColorAttachmentWrite;   // <-- include color attachment write for resolve

        barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

        srcStageFlags =
            vk::PipelineStageFlagBits::eEarlyFragmentTests |
            vk::PipelineStageFlagBits::eLateFragmentTests |
            vk::PipelineStageFlagBits::eColorAttachmentOutput;  // <-- include color attachment output stage

        dstStageFlags = vk::PipelineStageFlagBits::eFragmentShader;
    }
    else
    {
        std::string currentLayoutStr = ImageLayoutToString(currentLayout);
        std::string targetLayoutStr  = ImageLayoutToString(targetLayout);
        auto errormsg = "Transition from " + currentLayoutStr + " layout to " + targetLayoutStr + " layout is not yet supported";
        throw std::runtime_error(errormsg);
    }
}

std::string VulkanUtils::ImageLayoutToString(vk::ImageLayout imageLayout)
{
    switch(imageLayout)
    {
        case vk::ImageLayout::eUndefined:
            return "Undefined";
        case vk::ImageLayout::eGeneral:
            return "General";
        case vk::ImageLayout::eColorAttachmentOptimal:
            return "ColorAttachmentOptimal";
        case vk::ImageLayout::eDepthStencilAttachmentOptimal:
            return "DepthStencilAttachmentOptimal";
        case vk::ImageLayout::eDepthStencilReadOnlyOptimal:
            return "DepthStencilReadOnlyOptimal";
        case vk::ImageLayout::eShaderReadOnlyOptimal:
            return "ShaderReadOnlyOptimal";
        case vk::ImageLayout::eTransferSrcOptimal:
            return "TransferSrcOptimal";
        case vk::ImageLayout::eTransferDstOptimal:
            return "TransferDstOptimal";
        case vk::ImageLayout::ePreinitialized:
            return "Preinitialized";
        case vk::ImageLayout::eDepthReadOnlyStencilAttachmentOptimal:
            return "DepthReadOnlyStencilAttachmentOptimal";
        case vk::ImageLayout::eDepthAttachmentStencilReadOnlyOptimal:
            return "DepthAttachmentStencilReadOnlyOptimal";
        case vk::ImageLayout::eDepthAttachmentOptimal:
            return "DepthAttachmentOptimal";
        case vk::ImageLayout::eDepthReadOnlyOptimal:
            return "DepthReadOnlyOptimal";
        case vk::ImageLayout::eStencilAttachmentOptimal:
            return "StencilAttachmentOptimal";
        case vk::ImageLayout::eStencilReadOnlyOptimal:
            return "StencilReadOnlyOptimal";
        case vk::ImageLayout::ePresentSrcKHR:
            return "PresentSrcKHR";
        case vk::ImageLayout::eSharedPresentKHR:
            return "SharedPresentKHR";
        case vk::ImageLayout::eFragmentDensityMapOptimalEXT:
            return "FragmentDensityMapOptimalEXT";
        case vk::ImageLayout::eFragmentShadingRateAttachmentOptimalKHR:
            return "FragmentShadingRateAttachmentOptimalKHR";
        default:
            return "Unknown ImageLayout";
    }
}
