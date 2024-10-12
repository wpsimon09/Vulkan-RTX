//
// Created by wpsimon09 on 12/10/24.
//

#include "VFrameBuffer.hpp"

#include "Vulkan/VulkanCore/Device/VDevice.hpp"
#include "Vulkan/VulkanCore/RenderPass/VRenderPass.hpp"
#include "Vulkan/VulkanCore/SwapChain/VSwapChain.hpp"

VulkanCore::VFrameBuffer::VFrameBuffer(const VDevice &device, const VRenderPass &renderPass,
                                       const VSwapChain &swapChain): m_device(device) {

    vk::FramebufferCreateInfo createFrameBufferInfo;
    createFrameBufferInfo.renderPass = renderPass.GetRenderPass();
    createFrameBufferInfo.height = swapChain.GetExtent().height;
    createFrameBufferInfo.width = swapChain.GetExtent().width;
    createFrameBufferInfo.pAttachments = swapChain.GetImageViews().data();
    createFrameBufferInfo.attachmentCount = swapChain.GetImageViews().size();
    createFrameBufferInfo.layers = 1;

    m_frameBuffer = m_device.GetDevice().createFramebuffer(createFrameBufferInfo);
    assert(m_frameBuffer);
}
