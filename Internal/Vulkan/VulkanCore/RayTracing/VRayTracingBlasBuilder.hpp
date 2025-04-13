//
// Created by wpsimon09 on 13/04/25.
//

#ifndef VRAYTRACINGBLASBUILDER_HPP
#define VRAYTRACINGBLASBUILDER_HPP
#include "VRayTracingStructs.hpp"


#include <vulkan/vulkan.hpp>

namespace VulkanCore::RTX {

struct AccelerationStructBuildData;
}  // namespace VulkanCore::RTX

namespace VulkanCore {
class VCommandBuffer;
}

namespace VulkanCore {
class VDevice;
}

namespace VulkanCore {
namespace RTX {

class VRayTracingBlasBuilder
{
public:
  explicit VRayTracingBlasBuilder(const VulkanCore::VDevice& device);

  bool CmdCreateBlas(const VulkanCore::VCommandBuffer&                          cmdBuffer,
                     std::vector<VulkanCore::RTX::AccelerationStructBuildData>& buildInfo,
                     std::vector<VulkanCore::RTX::AccelKHR>&                    outAs,
                     vk::DeviceAddress                                          scratchAdress,
                     vk::DeviceSize                                             hintMaxBudget = 512'000'000);

  bool CmdCreateParallelBlas( const VulkanCore::VCommandBuffer&                          cmdBuffer,
                              std::vector<VulkanCore::RTX::AccelerationStructBuildData>& buildInfo,
                              std::vector<VulkanCore::RTX::AccelKHR>&                    outAs,
                              std::vector<vk::DeviceAddress>&                            scratchAdresses,
                              vk::DeviceSize                                             hintMaxBudget = 512'000'000);

      void CmdCompactBlas(const VulkanCore::VCommandBuffer&         cmdBuffer,
                          std::vector<AccelerationStructBuildData>& blasBuildData,
                          std::vector<AccelKHR>&                    outBlas);

  void DestroyNonCompactedBlas();

  vk::DeviceSize GetScratchSize(vk::DeviceSize                                  hintMaxBudget,
                                const std::vector<AccelerationStructBuildData>& blasBuildData,
                                uint32_t                                        minAlignment);

  void GetScratchAddresses(vk::DeviceSize                                  hintMaxBudget,
                           const std::vector<AccelerationStructBuildData>& blasBuildData,
                           vk::DeviceAddress                               scratchBufferAderess,
                           std::vector<vk::DeviceAddress>&                 outScratchAddresses,
                           uint32_t                                        minimumAligment);


  void Destroy();

private:
  void           DestroyQueryPool();
  void           CreateQueryPool(uint32_t maxBlasCount);
  void           InitializeQueryPoolIfNeeded(const std::vector<AccelerationStructBuildData>& blasBuildData);
  vk::DeviceSize BuildAccelerationStructures(const VulkanCore::VCommandBuffer&         cmdBuffer,
                                             std::vector<AccelerationStructBuildData>& blasBuildData,
                                             std::vector<AccelKHR>&                    outAccel,
                                             const std::vector<vk::DeviceAddress>&     scratchAdress,
                                             vk::DeviceSize                            hintMaxBudget,
                                             vk::DeviceSize                            currentBudget,
                                             uint32_t&                                 currentQueryIndex);

private:
  const VulkanCore::VDevice& m_device;
  vk::QueryPool              m_queryPool;
  uint32_t                   m_currentBlasIndex{0};
  uint32_t                   m_currentQueryIndex{0};

  std::vector<AccelKHR> m_cleanUpdBlasAccell;
};

}  // namespace RTX
}  // namespace VulkanCore

#endif  // VRAYTRACINGBLASBUILDER_HPP
