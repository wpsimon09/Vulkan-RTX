//
// Created by wpsimon09 on 20/04/25.
//

#ifndef RAYTRACER_HPP
#define RAYTRACER_HPP
#include <memory>
#include <vector>


namespace VulkanUtils {
class VEffect;
}
namespace ApplicationCore {
class EffectsLibrary;
}
struct SceneUpdateContext;
namespace VulkanUtils {
class VRasterEffect;
}
namespace ApplicationCore {
struct SceneData;
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
class VDescriptorLayoutCache;
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
    RayTracer(const VulkanCore::VDevice&           device,
              ApplicationCore::EffectsLibrary&     effectsLibrary,
              VulkanUtils::VRayTracingDataManager& rtxDataManager,
              int                                  width,
              int                                  height);

    void TraceRays(const VulkanCore::VCommandBuffer&         cmdBuffer,
                   const VulkanUtils::VUniformBufferManager& unifromBufferManager,
                   int                                       currentFrame);

    void ProcessResize(int newWidth, int newHeight);

    VulkanCore::VImage2& GetRenderedImage(int currentFrameIndex);

    void Destroy();

  private:
    const VulkanCore::VDevice&           m_device;
    VulkanUtils::VRayTracingDataManager& m_rtxDataManager;

    std::shared_ptr<VulkanUtils::VEffect>       m_rtxEffect;
    std::unique_ptr<VulkanUtils::VRasterEffect> m_accumulationEffect;

    std::vector<std::unique_ptr<VulkanCore::VImage2>> m_resultImage;
    std::unique_ptr<VulkanCore::VImage2>              m_accumulationResultImage;
};

}  // namespace Renderer

#endif  //RAYTRACER_HPP
