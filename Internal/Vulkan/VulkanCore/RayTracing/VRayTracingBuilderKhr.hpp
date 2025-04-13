//
// Created by wpsimon09 on 12/04/25.
//

#ifndef VRAYTRACINGBUILDERKHR_HPP
#define VRAYTRACINGBUILDERKHR_HPP
#include <memory>

#include "vulkan/vulkan.hpp"

#include "VRayTracingStructs.hpp"

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

private:
  const VulkanCore::VDevice&                  m_device;
  std::vector<RTX::BLASEntry>                 m_blasEntries;
  std::unique_ptr<VulkanCore::VCommandPool>   m_cmdPool;
  std::unique_ptr<VulkanCore::VCommandBuffer> m_cmdBuffer;
};
}  // namespace VulkanCore::RTX

#endif  //VRAYTRACINGBUILDERKHR_HPP
