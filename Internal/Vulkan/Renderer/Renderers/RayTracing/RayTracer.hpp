//
// Created by wpsimon09 on 20/04/25.
//

#ifndef RAYTRACER_HPP
#define RAYTRACER_HPP
#include <memory>
#include <vector>

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
}
namespace VulkanCore {
class VDevice;
}
namespace Renderer {

class RayTracer {
public:
  RayTracer(const VulkanCore::VDevice& device,VulkanUtils::VRayTracingDataManager& rtxDataManager, int width, int height);

  void TraceRays(const VulkanCore::VCommandBuffer& cmdBuffer,const VulkanCore::VTimelineSemaphore& renderingSemaphore,  VulkanUtils::VUniformBufferManager& unifromBufferManager, int currentFrame);

  void ProcessResize(int newWidth, int newHeight);

  void Destroy();
private :
  const VulkanCore::VDevice& m_device;
  VulkanUtils::VRayTracingDataManager& m_rtxDataManager;

  std::vector<std::unique_ptr<VulkanCore::VImage2>> m_resultImage;
};

} // Renderer

#endif //RAYTRACER_HPP
