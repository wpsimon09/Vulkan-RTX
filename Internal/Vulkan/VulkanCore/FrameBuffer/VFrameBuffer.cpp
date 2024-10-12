//
// Created by wpsimon09 on 12/10/24.
//

#include "VFrameBuffer.hpp"

#include "Vulkan/Utils/VImage/VImage.hpp"
#include "Vulkan/VulkanCore/Device/VDevice.hpp"
#include "Vulkan/VulkanCore/RenderPass/VRenderPass.hpp"

VulkanCore::VFrameBuffer::VFrameBuffer(const VDevice &device, const VRenderPass &renderPass,
    const std::vector<VulkanUtils::VImage> attachments, uint32_t width, uint32_t height):m_device(device) {

    std::vector<vk::ImageView> retreivedAttachments(attachments.size());
    assert(retreivedAttachments.size() == attachments.size());
    for (size_t i = 0; i < retreivedAttachments.size(); i++) {
        retreivedAttachments[i] = attachments[i].GetImageView();
    }

    vk::FramebufferCreateInfo createFrameBufferInfo;
    createFrameBufferInfo.renderPass =          renderPass.GetRenderPass();
    createFrameBufferInfo.height =              height;
    createFrameBufferInfo.width =               width;
    createFrameBufferInfo.attachmentCount = retreivedAttachments.size();
    createFrameBufferInfo.pAttachments = retreivedAttachments.data();
    createFrameBufferInfo.layers = 1;

    m_frameBuffer = m_device.GetDevice().createFramebuffer(createFrameBufferInfo);
    assert(m_frameBuffer);
}
