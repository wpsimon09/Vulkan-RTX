//
// Created by wpsimon09 on 20/06/25.
//

#include "RenderTarget2.h"

#include "Vulkan/Utils/VPipelineBarriers.hpp"
#include "Vulkan/Utils/TransferOperationsManager/VTransferOperationsManager.hpp"
#include "Vulkan/VulkanCore/SwapChain/VSwapChain.hpp"
#include "Vulkan/VulkanCore/VImage/VImage2.hpp"

namespace Renderer {
RenderTarget2::RenderTarget2(const VulkanCore::VDevice& device, RenderTarget2CreatInfo& m_renderTargetInfo)
    : m_device(device)
    , m_renderTargetInfo(m_renderTargetInfo)
{
    //==================================================
    // Creat main attachment image
    //==================================================
    VulkanCore::VImage2CreateInfo attachemtImageCI;
    attachemtImageCI.format = m_renderTargetInfo.format;
    attachemtImageCI.height = m_renderTargetInfo.heigh;
    attachemtImageCI.width  = m_renderTargetInfo.width;
    attachemtImageCI.layout = m_renderTargetInfo.initialLayout;
    attachemtImageCI.samples = m_renderTargetInfo.multiSampled ? m_device.GetSampleCount() : vk::SampleCountFlagBits::e1;
    attachemtImageCI.mipLevels      = 1;
    attachemtImageCI.imageDebugName = m_renderTargetInfo.imageDebugName;
    attachemtImageCI.aspecFlags = m_renderTargetInfo.isDepth ? vk::ImageAspectFlagBits::eDepth : vk::ImageAspectFlagBits::eColor;
    if(m_renderTargetInfo.isDepth)
    {
        assert(!m_renderTargetInfo.computeShaderOutput && "Depth image can not be storage image ");
        attachemtImageCI.imageDebugName = m_renderTargetInfo.imageDebugName + " | depth-stencil attachment";
        attachemtImageCI.imageUsage |= vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled;
    }
    else if(m_renderTargetInfo.computeShaderOutput)
    {
        assert(!m_renderTargetInfo.isDepth && "Storage image can not be depth buffer");
        attachemtImageCI.imageDebugName = m_renderTargetInfo.imageDebugName + " | compute shader attachment";
        attachemtImageCI.imageUsage |= vk::ImageUsageFlagBits::eStorage;
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

    vk::ImageLayout initialLayout = m_renderTargetInfo.initialLayout;
    // since barrier has to be either for colour or depth attachment, for evaluating its position
    // i have to define weather it is for depth or for colour
    // other than that i can use new vk::ImageLayout::eAttachmentOptimal
    if(m_renderTargetInfo.initialLayout == vk::ImageLayout::eAttachmentOptimal)
    {
        if(m_renderTargetInfo.isDepth)
        {
            initialLayout = vk::ImageLayout::eDepthStencilAttachmentOptimal;
        }
        else
        {
            initialLayout = vk::ImageLayout::eColorAttachmentOptimal;
        }
    }
    if(m_renderTargetInfo.initialLayout != vk::ImageLayout::eUndefined)
    {

        auto barrierPos = VulkanUtils::EvaluateBarrierPositionFromUndefinedLayout(initialLayout);
        VulkanUtils::PlaceImageMemoryBarrier2(*m_primaryAttachment, cmdBuffer, vk::ImageLayout::eUndefined,
                                              m_renderTargetInfo.initialLayout, barrierPos);
    }


    //=================================================
    // Create resolve target image
    //=================================================
    if(m_renderTargetInfo.multiSampled && m_renderTargetInfo.resolveMode != vk::ResolveModeFlagBits::eNone)
    {
        attachemtImageCI.samples = vk::SampleCountFlagBits::e1;
        attachemtImageCI.imageDebugName += " | resolved attachment ";
        m_resolvedAttachment = std::make_unique<VulkanCore::VImage2>(m_device, attachemtImageCI);

        //===================================
        // transition to specified layout
        //===================================
        if(m_renderTargetInfo.initialLayout != vk::ImageLayout::eUndefined)
        {
            VulkanUtils::PlaceImageMemoryBarrier2(*m_resolvedAttachment, cmdBuffer, vk::ImageLayout::eUndefined,
                                                  m_renderTargetInfo.initialLayout,
                                                  VulkanUtils::EvaluateBarrierPositionFromUndefinedLayout(initialLayout));
        }
    }
}


RenderTarget2::RenderTarget2(const VulkanCore::VDevice& device, const VulkanCore::VSwapChain& swapChain)
    : m_device(device)
    , m_isForSwapChain(true)
{

    m_swapChainImages.resize(swapChain.GetImageCount());
    auto& swapChainImages = swapChain.GetSwapChainImages();
    auto& swapChainExtent = swapChain.GetExtent();
    auto& swapChainFormat = swapChain.GetSurfaceFormatKHR().format;

    m_renderTargetInfo = {
        static_cast<int>(swapChainExtent.width),
        static_cast<int>(swapChainExtent.height),
        false,
        false,
        swapChainFormat,
        vk::ImageLayout::ePresentSrcKHR,
        vk::ResolveModeFlagBits::eNone,
    };

    //==========================
    // CREATE COLOUR ATTACHMENT
    //==========================
    VulkanCore::VImage2CreateInfo colourAttachmentCreateInfo;
    colourAttachmentCreateInfo.format     = swapChainFormat;
    colourAttachmentCreateInfo.height     = swapChainExtent.height;
    colourAttachmentCreateInfo.width      = swapChainExtent.width;
    colourAttachmentCreateInfo.mipLevels  = 1;
    colourAttachmentCreateInfo.aspecFlags = vk::ImageAspectFlagBits::eColor;

    for(int i = 0; i < swapChainImages.size(); i++)
    {
        m_swapChainImages[i] = std::make_unique<VulkanCore::VImage2>(m_device, colourAttachmentCreateInfo, swapChainImages[i]);

        VulkanUtils::PlaceImageMemoryBarrier2(*m_swapChainImages[i], device.GetTransferOpsManager().GetCommandBuffer(),
                                              vk::ImageLayout::eUndefined, vk::ImageLayout::ePresentSrcKHR,
                                              VulkanUtils::VImage_Undefined_ToPresent);
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
vk::RenderingAttachmentInfo RenderTarget2::GenerateAttachmentInfoForSwapChain(int swapChainImageIndex)
{
    assert(m_isForSwapChain && "Render target must be created with constructor that accepts swap chain as a parameter ! ");
    vk::RenderingAttachmentInfo attachmentInfo(m_swapChainImages[swapChainImageIndex]->GetImageView(),
                                               vk::ImageLayout::eColorAttachmentOptimal, vk::ResolveModeFlagBits::eNone,
                                               {}, {},  // resolveImageView, resolveImageLayout
                                               vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore,
                                               vk::ClearValue{vk::ClearColorValue{std::array<float, 4>{0.0f, 0.0f, 0.0f, 1.0f}}});

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
VulkanCore::VImage2& RenderTarget2::GetSwapChainImage(int index) const
{
    assert(m_isForSwapChain && "Render target is not for swap chain !");
    return *m_swapChainImages[index];
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

void RenderTarget2::TransitionAttachments(VulkanCore::VCommandBuffer&          cmdBuffer,
                                          vk::ImageLayout                      targetLayout,
                                          vk::ImageLayout                      oldLayout,
                                          const VulkanUtils::VBarrierPosition& barrierPosition)
{
    VulkanUtils::PlaceImageMemoryBarrier2(*m_primaryAttachment, cmdBuffer, oldLayout, targetLayout, barrierPosition);
    if(m_resolvedAttachment)
    {
        VulkanUtils::PlaceImageMemoryBarrier2(*m_resolvedAttachment, cmdBuffer, oldLayout, targetLayout, barrierPosition);
    }
}

bool RenderTarget2::IsForSwapChain() const
{
    return m_isForSwapChain;
}

void RenderTarget2::Destroy()
{
    if(m_primaryAttachment)
    {
        m_primaryAttachment->Destroy();
    }
    if(m_resolvedAttachment)
        m_resolvedAttachment->Destroy();

    if(!m_swapChainImages.empty())
    {
        for(const auto& swapChainImage : m_swapChainImages)
        {
            swapChainImage->Destroy();
        }
    }
}

RenderTarget2::~RenderTarget2() {}

}  // namespace Renderer