//
// Created by wpsimon09 on 16/08/2025.
//

#ifndef RENDERPASS_HPP
#define RENDERPASS_HPP
#include "Vulkan/Global/VulkanStructs.hpp"
#include "Vulkan/VulkanCore/Descriptors/VDescriptorAllocator.hpp"


#include <vector>


namespace VulkanUtils {
struct RenderContext;
class VRayTracingDataManager;
class VUniformBufferManager;
}
namespace Renderer {
class RenderTarget2;
}
namespace VulkanCore {
class VCommandBuffer;
class VImage2;
class VDevice;
}
namespace Renderer {


class RenderPass
{
  public:
    RenderPass(const VulkanCore::VDevice& device, int width, int height);

    virtual void Init(int                                 currentFrameIndex,
                      VulkanUtils::VUniformBufferManager& uniformBufferManager,
                      VulkanUtils::RenderContext*         renderContext)    = 0;

    virtual void Update(int                                   currentFrame,
                        VulkanUtils::VUniformBufferManager&   uniformBufferManager,
                        VulkanUtils::RenderContext*           renderContext,
                        VulkanStructs::PostProcessingContext* postProcessingContext)  = 0;

    virtual void Render(int currentFrame,VulkanCore::VCommandBuffer& cmdBuffer,  VulkanUtils::RenderContext* renderContext )  = 0;
    virtual void Destroy();

    RenderTarget2& GetRenderTarget(int index = 0);
    VulkanCore::VImage2& GetResolvedResult(int index = 0);
    VulkanCore::VImage2& GetPrimaryResult(int index = 0);

  protected:
    const VulkanCore::VDevice& m_device;
    std::vector<std::unique_ptr<Renderer::RenderTarget2>> m_renderTargets;
    int m_width, m_height;
};

}  // namespace Renderer

#endif  //VULKAN_RTX_RENDERPASS_HPP
