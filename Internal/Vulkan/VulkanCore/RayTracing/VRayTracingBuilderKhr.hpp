//
// Created by wpsimon09 on 12/04/25.
//

#ifndef VRAYTRACINGBUILDERKHR_HPP
#define VRAYTRACINGBUILDERKHR_HPP
#include <memory>

#include "vulkan/vulkan.hpp"

#include "VRayTracingStructs.hpp"
#include "Vulkan/VulkanCore/Synchronization/VTimelineSemaphore.hpp"

namespace VulkanCore {
class VCommandBuffer;
}

namespace ApplicationCore {
class Scene;
}

namespace VulkanCore {
class VCommandPool;
class VDevice;
}  // namespace VulkanCore

namespace VulkanCore::RTX {
// each mesh will be stored in one of those

/**
     * This class will act as a Top level AS according to NVIDIA tutorial, so i might redo it later
     *
     */
class VRayTracingBuilderKHR
{
  public:
    explicit VRayTracingBuilderKHR(const VulkanCore::VDevice& device);
    void BuildBLAS(std::vector<RTX::BLASInput>& inputs,
                   vk::BuildAccelerationStructureFlagsKHR flags = vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace);

    void BuildTLAS(const std::vector<vk::AccelerationStructureInstanceKHR>& instances,
                   vk::BuildAccelerationStructureFlagsKHR flags = vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace,
                   bool update = false,
                   bool motion = false);

    vk::DeviceAddress GetInstanceDeviceAddress(uint32_t instance) const;

    const vk::AccelerationStructureKHR& GetTLAS() const;

    void Destroy();
    void Clear();

  private:
    VTimelineSemaphore m_asBuildSemaphore;


    const VulkanCore::VDevice&  m_device;
    std::vector<RTX::BLASEntry> m_blasEntries;

    std::unique_ptr<VulkanCore::VCommandPool>   m_cmdPool;
    std::unique_ptr<VulkanCore::VCommandBuffer> m_cmdBuffer;

    VulkanCore::RTX::AccelKHR              m_tlas;
    std::vector<VulkanCore::RTX::AccelKHR> m_blas;

  private:
    void CmdCreteTlas(const vk::CommandBuffer&               cmdBuffer,
                      uint32_t                               numInstances,
                      vk::DeviceAddress                      instancesDataBuffer,
                      VulkanCore::VBuffer&                   scratchBuffer,
                      vk::BuildAccelerationStructureFlagsKHR flags,
                      bool                                   update,
                      bool                                   motion);
};
}  // namespace VulkanCore::RTX

#endif  //VRAYTRACINGBUILDERKHR_HPP
