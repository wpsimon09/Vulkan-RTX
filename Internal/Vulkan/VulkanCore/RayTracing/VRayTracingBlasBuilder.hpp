//
// Created by wpsimon09 on 13/04/25.
//

#ifndef VRAYTRACINGBLASBUILDER_HPP
#define VRAYTRACINGBLASBUILDER_HPP
#include <vulkan/vulkan.hpp>

namespace VulkanCore::RTX {
struct AccelKHR;
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
  VRayTracingBlasBuilder(const VulkanCore::VDevice& device);

  bool CmdCreateBlas(const VulkanCore::VCommandBuffer&                          cmdBuffer,
                     std::vector<VulkanCore::RTX::AccelerationStructBuildData>& buildInfo,
                     std::vector<VulkanCore::RTX::AccelKHR>&                    outAs,
                     vk::DeviceAddress                                          scratchAdress,
                     vk::DeviceSize                                             hintMaxBudget = 512'000'000);

  void CmdCompactBlas(const VulkanCore::VCommandBuffer&         cmdBuffer,
                      std::vector<AccelerationStructBuildData>& blasBuildData,
                      std::vector<AccelKHR>&                    outBlas);

  void DestroyNonCompactedBlas();

  vk::DeviceSize GetScratchSize(vk::DeviceSize                                  hintMaxBudget,
                                const std::vector<AccelerationStructBuildData>& blasBuildData,
                                uint32_t                                        minAlignment);

  void GetScratchAddresses(vk::DeviceSize hintMaxBudget, const std::vector<AccelerationStructBuildData>& blasBuildData, vk::DeviceAddress scratchBufferAdress, std::vector<vk::DeviceAddress>& scratchAddresses, uint32_t minimumAlligment);


  void Destroy();
};

}  // namespace RTX
}  // namespace VulkanCore

#endif  // VRAYTRACINGBLASBUILDER_HPP
