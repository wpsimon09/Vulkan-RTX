//
// Created by wpsimon09 on 20/06/25.
//

#include "RenderTarget2.h"

#include "Vulkan/Renderer/RenderTarget/RenderTarget.hpp"
#include "Vulkan/Utils/VPipelineBarriers.hpp"
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
    attachemtImageCI.layout    = createInfo.initialLayout;
    attachemtImageCI.samples   = createInfo.multiSampled ? m_device.GetSampleCount() : vk::SampleCountFlagBits::e1;
    attachemtImageCI.mipLevels = 1;
    attachemtImageCI.aspecFlags = createInfo.isDepth ? vk::ImageAspectFlagBits::eDepth : vk::ImageAspectFlagBits::eColor;
    if(createInfo.isDepth)
    {
        assert(!createInfo.computeShaderOutput && "Depth image can not be storage image ");
        attachemtImageCI.imageUsage |= vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled;
    }
    else if (createInfo.computeShaderOutput) {
        assert(!createInfo.isDepth && "Storage image can not be depth buffer");
        attachemtImageCI.imageUsage |= vk::ImageUsageFlagBits::eStorage ;
        attachemtImageCI.isStorage = true;
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

    vk::ImageLayout initialLayout = createInfo.initialLayout;
    // since barrier has to be either for colour or depth attachment, for evaluating its position
    // i have to define weather it is for depth or for colour
    // other than that i can use new vk::ImageLayout::eAttachmentOptimal
    if (createInfo.initialLayout == vk::ImageLayout::eAttachmentOptimal) {
        if (createInfo.isDepth){ initialLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal; }
        else {initialLayout = vk::ImageLayout::eColorAttachmentOptimal;}
    }
    if (createInfo.initialLayout != vk::ImageLayout::eUndefined) {

        auto barrierPos = VulkanUtils::EvaluateBarrierPositionFromUndefinedLayout(initialLayout);
        VulkanUtils::PlaceImageMemoryBarrier2(*m_primaryAttachment, cmdBuffer, vk::ImageLayout::eUndefined, createInfo.initialLayout, barrierPos );
    }


    //=================================================
    // Create resolve target image
    //=================================================
    if(createInfo.multiSampled && createInfo.resolveMode != vk::ResolveModeFlagBits::eNone)
    {
        attachemtImageCI.samples = vk::SampleCountFlagBits::e1;
        m_resolvedAttachment     = std::make_unique<VulkanCore::VImage2>(m_device, attachemtImageCI);

        //===================================
        // transition to specified layout
        //===================================
        if (createInfo.initialLayout != vk::ImageLayout::eUndefined) {
            VulkanUtils::PlaceImageMemoryBarrier2(*m_resolvedAttachment, cmdBuffer, vk::ImageLayout::eUndefined, createInfo.initialLayout, VulkanUtils::EvaluateBarrierPositionFromUndefinedLayout(initialLayout));
        }
    }
}
vk::RenderingAttachmentInfo RenderTarget2::GenerateAttachmentInfo(vk::ImageLayout layout, vk::AttachmentLoadOp loadOp, vk::AttachmentStoreOp storeOp)
{
    bool                        shouldResolve = m_renderTargetInfo.resolveMode != vk::ResolveModeFlagBits::eNone;
    vk::RenderingAttachmentInfo attachmentInfo;
    attachmentInfo.imageView   = m_primaryAttachment->GetImageView();
    attachmentInfo.imageLayout = layout;
    attachmentInfo.resolveMode = m_renderTargetInfo.resolveMode;

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
vk::RenderingAttachmentInfo RenderTarget2::GenerateAttachmentInfoFromResolvedImage(vk::ImageLayout       layout,
                                                                                   vk::AttachmentLoadOp  loadOp,
                                                                                   vk::AttachmentStoreOp storeOp)
{
    if(m_resolvedAttachment == nullptr)
        throw std::runtime_error("This attachemtn does not have resovled output ! ");

    vk::RenderingAttachmentInfo attachmentInfo;
    attachmentInfo.imageView                       = m_resolvedAttachment->GetImageView();
    attachmentInfo.imageLayout                     = layout;
    attachmentInfo.resolveMode                     = vk::ResolveModeFlagBits::eNone;
    attachmentInfo.loadOp                          = loadOp;
    attachmentInfo.storeOp                         = storeOp;
    attachmentInfo.clearValue.color                = {0.0f, 0.0f, 0.0f, 1.0f};
    attachmentInfo.clearValue.depthStencil.depth   = 1.0f;
    attachmentInfo.clearValue.depthStencil.stencil = 0.0f;

    return attachmentInfo;
}

VulkanCore::VImage2& RenderTarget2::GetPrimaryImage() const
{
    return *m_primaryAttachment;
}
VulkanCore::VImage2& RenderTarget2::GetResolvedImage() const
{
    return *m_resolvedAttachment;
}

uint32_t RenderTarget2::GetWidth()
{
    return m_renderTargetInfo.width;
}
uint32_t RenderTarget2::GetHeight()
{
    return m_renderTargetInfo.heigh;
}

vk::ImageView RenderTarget2::GetPrimaryImageView() const
{
    return m_primaryAttachment->GetImageView();
}
vk::ImageView RenderTarget2::GetResolvedImageView() const
{
    return m_resolvedAttachment->GetImageView();
}
void RenderTarget2::TransitionAttachments(VulkanCore::VCommandBuffer& cmdBuffer, vk::ImageLayout targetLayout, vk::ImageLayout oldLayout) const
{
    VulkanUtils::RecordImageTransitionLayoutCommand(*m_primaryAttachment, targetLayout, oldLayout, cmdBuffer);
    if(m_resolvedAttachment)
    {
        VulkanUtils::RecordImageTransitionLayoutCommand(*m_resolvedAttachment, targetLayout, oldLayout, cmdBuffer);
    }
}
void RenderTarget2::TransitionAttachments(VulkanCore::VCommandBuffer&   cmdBuffer,
                                          vk::ImageLayout               targetLayout,
                                          vk::ImageLayout               oldLayout,
                                          const VulkanUtils::VBarrierPosition& barrierPosition)
{
    VulkanUtils::PlaceImageMemoryBarrier2(*m_primaryAttachment, cmdBuffer, oldLayout, targetLayout, barrierPosition);
    if(m_resolvedAttachment)
    {
        VulkanUtils::PlaceImageMemoryBarrier2(*m_resolvedAttachment, cmdBuffer, oldLayout, targetLayout, barrierPosition);
    }
}

void RenderTarget2::Destroy()
{
    m_primaryAttachment->Destroy();
    if(m_resolvedAttachment)
        m_resolvedAttachment->Destroy();
}


RenderTarget2::~RenderTarget2() {}

}  // namespace Renderer