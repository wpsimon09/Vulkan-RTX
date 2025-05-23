//
// Created by wpsimon09 on 12/10/24.
//

#ifndef FRAMEBUFFER_HPP
#define FRAMEBUFFER_HPP

#include "Vulkan/VulkanCore/Device/VDevice.hpp"
#include "Vulkan/VulkanCore/VObject.hpp"
#include <vulkan/vulkan.hpp>

namespace VulkanCore {
class VDevice;
class VImage;
class VRenderPass;
class VSwapChain;
/**
 * @deprecated This class is dpericated since rendering is using dynamic rendering thus no render pass and frame buffers are required
 */
class VFrameBuffer : public VObject
{
  public:
    VFrameBuffer(const VDevice&                                                device,
                 const VRenderPass&                                            renderPass,
                 std::vector<std::reference_wrapper<const VulkanCore::VImage>> attachments,
                 uint32_t                                                      width,
                 uint32_t                                                      height);

    const vk::Framebuffer& GetFrameBuffer() const { return m_frameBuffer; }

    void Destroy() override;

  private:
    const VDevice& m_device;

    vk::Framebuffer m_frameBuffer;
};
}  // namespace VulkanCore


#endif  //FRAMEBUFFER_HPP
