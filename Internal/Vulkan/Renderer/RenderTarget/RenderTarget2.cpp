//
// Created by wpsimon09 on 20/06/25.
//

#include "RenderTarget2.h"

#include "Vulkan/Utils/TransferOperationsManager/VTransferOperationsManager.hpp"
#include "Vulkan/VulkanCore/VImage/VImage2.hpp"

namespace Renderer {
RenderTarget2::RenderTarget2(const VulkanCore::VDevice& device, RenderTarget2CreatInfo& createInfo)
    : m_device(device)
    , m_renderTargetInfo(createInfo)
{

    //==================================================
    // Creat main attachment image
    //==================================================
    VulkanCore::VImage2CreateInfo attachemtImageCI;
    attachemtImageCI.format    = createInfo.format;
    attachemtImageCI.height    = createInfo.heigh;
    attachemtImageCI.width     = createInfo.width;
    attachemtImageCI.samples   = createInfo.multiSampled ? m_device.GetSampleCount() : vk::SampleCountFlagBits::e1;
    attachemtImageCI.mipLevels = 1;
    attachemtImageCI.aspecFlags = createInfo.isDepth ? vk::ImageAspectFlagBits::eDepth : vk::ImageAspectFlagBits::eColor;
    if(createInfo.isDepth)
    {
        attachemtImageCI.imageUsage = vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled;
    }
    else
    {
        attachemtImageCI.imageUsage = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled
                                      | vk::ImageUsageFlagBits::eInputAttachment;
    }

    m_primaryAttachment = std::make_unique<VulkanCore::VImage2>(m_device, attachemtImageCI);


    //===================================
    // transition to specified layout
    //===================================
    auto& cmdBuffer = m_device.GetTransferOpsManager().GetCommandBuffer();

    VulkanUtils::RecordImageTransitionLayoutCommand(*m_primaryAttachment, createInfo.initialLayout,
                                                    vk::ImageLayout::eUndefined, cmdBuffer.GetCommandBuffer());


    //=================================================
    // Create resolve target image
    //=================================================
    if(createInfo.multiSampled && createInfo.resolveMode != vk::ResolveModeFlagBits::eNone)
    {
        m_resolvedAttachment = std::make_unique<VulkanCore::VImage2>(m_device, attachemtImageCI);

        //===================================
        // transition to specified layout
        //===================================
        VulkanUtils::RecordImageTransitionLayoutCommand(*m_primaryAttachment, createInfo.initialLayout,
                                                        vk::ImageLayout::eUndefined, cmdBuffer.GetCommandBuffer());
    }
}
vk::RenderingAttachmentInfo RenderTarget2::GenerateAttachmentInfo(vk::ImageLayout layout, vk::AttachmentLoadOp loadOp, vk::AttachmentStoreOp storeOp)
{
    bool                        shouldResolve = m_renderingAttachmentInfo.resolveMode != vk::ResolveModeFlagBits::eNone;
    vk::RenderingAttachmentInfo attachmentInfo;
    attachmentInfo.imageView   = m_primaryAttachment->GetImageView();
    attachmentInfo.imageLayout = layout;
    attachmentInfo.resolveMode = m_renderingAttachmentInfo.resolveMode;

    if(shouldResolve && m_resolvedAttachment != nullptr)
    {
        attachmentInfo.resolveImageView   = m_resolvedAttachment->GetImageView();
        attachmentInfo.resolveImageLayout = layout;
    }

    attachmentInfo.loadOp                          = loadOp;
    attachmentInfo.storeOp                         = storeOp;
    attachmentInfo.clearValue.color                = {0.0f, 0.0f, 0.0f, 1.0f};
    attachmentInfo.clearValue.depthStencil.depth   = 1.0f;
    attachmentInfo.clearValue.depthStencil.stencil = 0.0f;

    return attachmentInfo;
}

VulkanCore::VImage2& RenderTarget2::GetPrimaryImage()
{
    return *m_primaryAttachment;
}
VulkanCore::VImage2& RenderTarget2::GetResolvedImage()
{
    return *m_resolvedAttachment;
}

vk::ImageView RenderTarget2::GetPrimaryImageView()
{
    return m_primaryAttachment->GetImageView();
}
vk::ImageView RenderTarget2::GetResolvedImageView()
{
    return m_resolvedAttachment->GetImageView();
}
void RenderTarget2::TransitionAttachments(VulkanCore::VCommandBuffer& cmdBuffer, vk::ImageLayout targetLayout, vk::ImageLayout oldLayout)
{
    VulkanUtils::RecordImageTransitionLayoutCommand(*m_primaryAttachment, targetLayout, oldLayout, cmdBuffer);
    if (m_resolvedAttachment) {
        VulkanUtils::RecordImageTransitionLayoutCommand(*m_resolvedAttachment, targetLayout, oldLayout, cmdBuffer);
    }
}

}  // namespace Renderer