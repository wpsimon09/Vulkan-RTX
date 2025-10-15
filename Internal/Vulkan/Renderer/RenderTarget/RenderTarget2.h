//
// Created by wpsimon09 on 20/06/25.
//

#ifndef RENDERTARGET2_H
#define RENDERTARGET2_H
#include "VulkanRtx.hpp"

namespace VulkanUtils {
struct VBarrierPosition;
}
namespace Renderer {

/**
 * NOTE: put layout undefined if you don`t want to transition the image during creation
 */
struct RenderTarget2CreatInfo
{
    int width;
    int heigh;

    bool multiSampled{false};
    bool isDepth{false};

    vk::Format              format{vk::Format::eR16G16B16A16Sfloat};
    vk::ImageLayout         initialLayout{vk::ImageLayout::eShaderReadOnlyOptimal};
    vk::ResolveModeFlagBits resolveMode{vk::ResolveModeFlagBits::eAverage};

    bool        computeShaderOutput{false};
    std::string imageDebugName = "";
};

class RenderTarget2
{
  public:
    RenderTarget2(const VulkanCore::VDevice& device, RenderTarget2CreatInfo& createInfo);
    RenderTarget2(const VulkanCore::VDevice& device, const VulkanCore::VSwapChain& swapChain);

    vk::RenderingAttachmentInfo GenerateAttachmentInfo(vk::ImageLayout layout, vk::AttachmentLoadOp loadOp, vk::AttachmentStoreOp storeOp);
    vk::RenderingAttachmentInfo GenerateAttachmentInfoFromResolvedImage(vk::ImageLayout       layout,
                                                                        vk::AttachmentLoadOp  loadOp,
                                                                        vk::AttachmentStoreOp storeOp);
    vk::RenderingAttachmentInfo GenerateAttachmentInfoForSwapChain(int swapChainImageIndex);

    VulkanCore::VImage2& GetPrimaryImage() const;
    VulkanCore::VImage2& GetResolvedImage() const;
    VulkanCore::VImage2& GetSwapChainImage(int index) const;

    uint32_t GetWidth();
    uint32_t GetHeight();

    vk::ImageView GetPrimaryImageView() const;
    vk::ImageView GetResolvedImageView() const;

    void TransitionAttachments(VulkanCore::VCommandBuffer&          cmdBuffer,
                               vk::ImageLayout                      targetLayout,
                               vk::ImageLayout                      oldLayout,
                               const VulkanUtils::VBarrierPosition& barrierPosition);

    void ResizeAttachments(int newWidth, int newHeight);

    bool IsForSwapChain() const;

    void Destroy();

    ~RenderTarget2();

  private:
    const VulkanCore::VDevice& m_device;

    vk::RenderingAttachmentInfo          m_renderingAttachmentInfo;
    std::unique_ptr<VulkanCore::VImage2> m_primaryAttachment;
    std::unique_ptr<VulkanCore::VImage2> m_resolvedAttachment;

    std::vector<std::unique_ptr<VulkanCore::VImage2>> m_swapChainImages;
    RenderTarget2CreatInfo                            m_renderTargetInfo;

    bool m_isForSwapChain = false;
};

}  // namespace Renderer

#endif  //RENDERTARGET2_H
