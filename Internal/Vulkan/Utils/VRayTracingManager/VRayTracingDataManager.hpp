//
// Created by wpsimon09 on 20/04/25.
//

#ifndef VRAYTRACINGMANAGER_HPP
#define VRAYTRACINGMANAGER_HPP
#include "Vulkan/VulkanCore/RayTracing/VRayTracingStructs.hpp"


#include <memory>
#include <vector>

namespace ApplicationCore {
class Scene;
}
namespace VulkanCore {
class VDevice;
}
namespace vk {
struct AccelerationStructureInstanceKHR;
}
namespace VulkanCore::RTX {
class VRayTracingBuilderKHR;
}
namespace VulkanUtils {

/**
 * Class that manages acceleration structures creation, updates etc.
 */
class VRayTracingDataManager {
  public:
    VRayTracingDataManager(const VulkanCore::VDevice& device);

    // updates transformations in TLAS
    void UpdateAS(std::vector<VulkanCore::RTX::BLASInput>& blasInputs);

    vk::DescriptorBufferInfo GetObjDescriptionBufferInfo();

    void Update(ApplicationCore::Scene& scene);

    // rebuilds every acceleration structures
    void InitAs(std::vector<VulkanCore::RTX::BLASInput>& blasInputs);
    void Destroy();
    const vk::AccelerationStructureKHR&  GetTLAS();
    vk::AccelerationStructureKHR         GetTLASCpy();

  private:
    const VulkanCore::VDevice& m_device;

    std::vector<RTXObjDescription> m_rtxObjectDescriptions;
    std::unique_ptr<VulkanCore::RTX::VRayTracingBuilderKHR> m_rayTracingBuilder;
    std::vector <VulkanCore::RTX::BLASInput> m_blasInputs;
    std::vector<vk::AccelerationStructureInstanceKHR> m_instances;

    std::unique_ptr<VulkanCore::VBuffer> m_objDescriptionBuffer;
};

} // VulkanUtils

#endif //VRAYTRACINGMANAGER_HPP
