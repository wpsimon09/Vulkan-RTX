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
class VTimelineSemaphore2;
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

    vk::DescriptorBufferInfo GetObjDescriptionBufferInfo();


    void UpdateData(SceneUpdateContext& sceneUpdateContext, std::vector<VulkanCore::RTX::BLASInput>& blasInputs);
    void RecordAndSubmitAsBuld(VulkanCore::VTimelineSemaphore2& frameSemaphore);
    void Destroy();
    const vk::AccelerationStructureKHR&  GetTLAS();
    vk::AccelerationStructureKHR         GetTLASCpy();


  private:
    const VulkanCore::VDevice& m_device;


    // rebuilds every acceleration structures
    void InitAs(std::vector<VulkanCore::RTX::BLASInput>& blasInputs, VulkanCore::VTimelineSemaphore2& frameSemaphore);

    std::vector<RTXObjDescription> m_rtxObjectDescriptions;
    std::unique_ptr<VulkanCore::RTX::VRayTracingBuilderKHR> m_rayTracingBuilder;
    std::vector <VulkanCore::RTX::BLASInput> m_blasInputs;
    std::vector<vk::AccelerationStructureInstanceKHR> m_instances;

    std::unique_ptr<VulkanCore::VBuffer> m_objDescriptionBuffer;
    SceneUpdateContext* m_sceneUpdateContext;

};

} // VulkanUtils

#endif //VRAYTRACINGMANAGER_HPP
