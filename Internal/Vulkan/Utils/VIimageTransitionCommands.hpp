#pragma once
#include "vulkan/vulkan.hpp"
#include "Vulkan/VulkanCore/CommandBuffer/VCommandBuffer.hpp"
inline void RecordImageTransitionLayoutComand(
    vk::ImageLayout currentLayout, 
    vk::ImageLayout targetLayout,
    vk::ImageMemoryBarrier& barrier,
    VulkanCore::VCommandBuffer& commandBuffer){

        vk::PipelineStageFlags srcStageFlags;
        vk::PipelineStageFlags dstStageFlags;
    
        if (currentLayout == vk::ImageLayout::eUndefined && targetLayout == vk::ImageLayout::eTransferDstOptimal) {
            barrier.srcAccessMask = {};
            barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
    
            srcStageFlags = vk::PipelineStageFlagBits::eTopOfPipe;
            dstStageFlags = vk::PipelineStageFlagBits::eTransfer;
        }
        else if (currentLayout == vk::ImageLayout::eTransferDstOptimal && targetLayout ==
            vk::ImageLayout::eShaderReadOnlyOptimal) {
            barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
            barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
    
            srcStageFlags = vk::PipelineStageFlagBits::eTransfer;
            dstStageFlags = vk::PipelineStageFlagBits::eFragmentShader;
        }
        else if (currentLayout == vk::ImageLayout::eUndefined && targetLayout ==
            vk::ImageLayout::eColorAttachmentOptimal) {
            barrier.srcAccessMask = {};
            barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;
    
            srcStageFlags = vk::PipelineStageFlagBits::eTopOfPipe;
            dstStageFlags = vk::PipelineStageFlagBits::eTransfer;
        }
        else if (currentLayout == vk::ImageLayout::eUndefined && targetLayout ==
            vk::ImageLayout::eDepthStencilAttachmentOptimal) {
            barrier.srcAccessMask = {};
            barrier.dstAccessMask = vk::AccessFlagBits::eDepthStencilAttachmentWrite;
    
            srcStageFlags = vk::PipelineStageFlagBits::eTopOfPipe;
            dstStageFlags = vk::PipelineStageFlagBits::eEarlyFragmentTests;
        }
        else if (currentLayout == vk::ImageLayout::eColorAttachmentOptimal && targetLayout ==
            vk::ImageLayout::eShaderReadOnlyOptimal) {
            barrier.srcAccessMask = vk::AccessFlagBits::eColorAttachmentWrite;
            barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
    
            srcStageFlags = vk::PipelineStageFlagBits::eColorAttachmentOutput;
            dstStageFlags = vk::PipelineStageFlagBits::eFragmentShader;
        }
        else if (currentLayout == vk::ImageLayout::eShaderReadOnlyOptimal && targetLayout ==
            vk::ImageLayout::eTransferSrcOptimal) {
            barrier.srcAccessMask = vk::AccessFlagBits::eShaderRead;
            barrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;
    
            srcStageFlags = vk::PipelineStageFlagBits::eFragmentShader;
            dstStageFlags = vk::PipelineStageFlagBits::eTransfer;
        }
        else if(currentLayout == vk::ImageLayout::eTransferSrcOptimal && targetLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
            barrier.srcAccessMask = vk::AccessFlagBits::eTransferRead;
            barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;
    
            srcStageFlags = vk::PipelineStageFlagBits::eTransfer;
            dstStageFlags = vk::PipelineStageFlagBits::eFragmentShader;
        }
        else {
            throw std::runtime_error("Unsupported layout transition");
        }

        commandBuffer.GetCommandBuffer().pipelineBarrier(
            srcStageFlags, dstStageFlags,
            {},
            0, nullptr,
            0, nullptr,
            1, &barrier
            );
    
    }