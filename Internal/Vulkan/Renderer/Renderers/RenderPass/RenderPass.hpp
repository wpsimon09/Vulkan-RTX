//
// Created by wpsimon09 on 16/08/2025.
//

#ifndef RENDERPASS_HPP
#define RENDERPASS_HPP
#include "Vulkan/Global/VulkanStructs.hpp"
#include "Vulkan/VulkanCore/Descriptors/VDescriptorAllocator.hpp"


#include <vector>


namespace VulkanStructs {
struct RenderContext;
}
namespace VulkanUtils {
class VUniformBufferManager;
}
namespace Renderer {
class RenderTarget2;
}
namespace VulkanCore {
class VImage2;
class VDevice;
}
namespace Renderer {

class RenderPass
{
  public:
    RenderPass(VulkanCore::VDevice& device, VulkanCore::VDescriptorLayoutCache& descLayoutCache,  int width, int height);

    virtual void Init(VulkanUtils::VUniformBufferManager& uniformBufferManager)    = 0;
    virtual void Update(int currentFrame, VulkanUtils::VUniformBufferManager& uniformBufferManager, VulkanStructs::RenderContext* renderContext = nullptr, VulkanStructs::PostProcessingContext* postProcessingContext = nullptr)  = 0;
    virtual void Render(int currentFrame, VulkanStructs::RenderContext* renderContext )  = 0;
    virtual void Destroy();

    RenderTarget2& GetRenderTarget(int index = 0);
    VulkanCore::VImage2& GetResolvedResult(int index = 0);
    VulkanCore::VImage2& GetPrimaryResult(int index = 0);

  private:
    std::vector<Renderer::RenderTarget2> m_renderTargets;
    VulkanCore::VDevice& m_device;
  protected:
    int m_width, m_height;
};

}  // namespace Renderer

#endif  //VULKAN_RTX_RENDERPASS_HPP
