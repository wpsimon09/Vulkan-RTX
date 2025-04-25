//
// Created by wpsimon09 on 20/04/25.
//

#ifndef RAYTRACER_HPP
#define RAYTRACER_HPP
#include <memory>
#include <vector>

namespace VulkanUtils {
class VResourceGroupManager;
}
namespace VulkanUtils {
class VRayTracingEffect;
}
namespace VulkanUtils {
class VRayTracingDataManager;
}
namespace VulkanUtils {
class VUniformBufferManager;
}
namespace VulkanCore {
class VTimelineSemaphore;
class VCommandBuffer;
class VImage2;
}  // namespace VulkanCore
namespace VulkanCore {
class VDevice;
}
namespace Renderer {

class RayTracer
{
  public:
    RayTracer(const VulkanCore::VDevice& device,
              VulkanUtils::VResourceGroupManager& resourceGroupManager,
              VulkanUtils::VRayTracingDataManager& rtxDataManager,
              int width,
              int height);

    void TraceRays(const VulkanCore::VCommandBuffer&         cmdBuffer,
                   const VulkanCore::VTimelineSemaphore&     renderingSemaphore,
                   const VulkanUtils::VUniformBufferManager& unifromBufferManager,
                   int                                       currentFrame);

    void ProcessResize(int newWidth, int newHeight);

    VulkanCore::VImage2 &  GetRenderedImage(int currentFrameIndex);

    void Destroy();

  private:
    const VulkanCore::VDevice&           m_device;
    VulkanUtils::VRayTracingDataManager& m_rtxDataManager;
    VulkanUtils::VResourceGroupManager& m_resourceGroupManager;

    std::unique_ptr<VulkanUtils::VRayTracingEffect> m_rtxEffect;

    std::vector<std::unique_ptr<VulkanCore::VImage2>> m_resultImage;
};

}  // namespace Renderer

#endif  //RAYTRACER_HPP
