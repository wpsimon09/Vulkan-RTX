//
// Created by wpsimon09 on 22/04/25.
//

#ifndef VRAYTRACINGEFFECT_HPP
#define VRAYTRACINGEFFECT_HPP
#include "VEffect.hpp"
#include "Vulkan/VulkanCore/RayTracing/VRayTracingStructs.hpp"
#include "Vulkan/VulkanCore/Shader/VRayTracingShaders.hpp"

namespace VulkanCore {
class VRayTracingShaders;
}
namespace VulkanCore::RTX {
class VRayTracingPipeline;
}
namespace VulkanUtils {

class VRayTracingEffect : public VEffect
{
  public:
    VRayTracingEffect(const VulkanCore::VDevice&             device,
                      const VulkanCore::RTX::RTXShaderPaths& shaderPaths,
                      const std::string&                     name,
                      VulkanCore::VDescriptorLayoutCache&    descLayoutCache);
    void                                  BuildEffect() override;
    vk::PipelineLayout                    GetPipelineLayout() override;
    VulkanCore::RTX::VRayTracingPipeline& GetRTXPipeline();
    void                                  BindPipeline(const vk::CommandBuffer& cmdBuffer) override;
    void                                  Destroy() override;
    void BindDescriptorSet(const vk::CommandBuffer& cmdBuffer, uint32_t frame, uint32_t set) override;

    vk::StridedDeviceAddressRegionKHR GetShaderBindingTableEntry(VulkanCore::RTX::ERayTracingStageIndices) override;

  private:
    std::unique_ptr<VulkanCore::RTX::VRayTracingPipeline> m_rtPipeline;
    VulkanCore::VRayTracingShaders                        m_shaders;
};

}  // namespace VulkanUtils

#endif  //VRAYTRACINGEFFECT_HPP
