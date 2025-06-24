//
// Created by wpsimon09 on 20/06/25.
//

#ifndef RENDERTARGET2_H
#define RENDERTARGET2_H
#include "VulkanRtx.hpp"

namespace Renderer {

struct RenderTarget2CreatInfo
{
    int                   width;
    int                   heigh;

    bool                  multiSampled{false};
    bool                  isDepth{false};

    vk::Format            format{vk::Format::eR16G16B16A16Sfloat};
    vk::ImageLayout       initialLayout {vk::ImageLayout::eShaderReadOnlyOptimal};
    vk::ResolveModeFlagBits  resolveMode{vk::ResolveModeFlagBits::eAverage};
};

class RenderTarget2
{
  public:
    RenderTarget2(const VulkanCore::VDevice& device, RenderTarget2CreatInfo& createInfo);
    RenderTarget2(const VulkanCore::VDevice& device, const VulkanCore::VSwapChain& swapChain);

    vk::RenderingAttachmentInfo GenerateAttachmentInfo(vk::ImageLayout layout, vk::AttachmentLoadOp loadOp, vk::AttachmentStoreOp storeOp);
    vk::RenderingAttachmentInfo GenerateAttachmentInfoFromResolvedImage(vk::ImageLayout layout, vk::AttachmentLoadOp loadOp, vk::AttachmentStoreOp storeOp);

    VulkanCore::VImage2& GetPrimaryImage() const;
    VulkanCore::VImage2& GetResolvedImage() const;

    vk::ImageView GetPrimaryImageView() const;
    vk::ImageView GetResolvedImageView() const;

    void TransitionAttachments(VulkanCore::VCommandBuffer& cmdBuffer, vk::ImageLayout targetLayout, vk::ImageLayout oldLayout) const;

    void Destroy();

    ~RenderTarget2();

  private:
    const VulkanCore::VDevice& m_device;

    vk::RenderingAttachmentInfo          m_renderingAttachmentInfo;
    std::unique_ptr<VulkanCore::VImage2> m_primaryAttachment;
    std::unique_ptr<VulkanCore::VImage2> m_resolvedAttachment;

    RenderTarget2CreatInfo m_renderTargetInfo;
};

}  // namespace Renderer

#endif  //RENDERTARGET2_H
