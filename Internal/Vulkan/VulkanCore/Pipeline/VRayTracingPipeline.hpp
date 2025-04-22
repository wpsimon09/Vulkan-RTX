//
// Created by wpsimon09 on 21/04/25.
//

#ifndef VRAYTRACINGPIPELINE_HPP
#define VRAYTRACINGPIPELINE_HPP
#include "Vulkan/VulkanCore/VObject.hpp"
#include "Vulkan/VulkanCore/RayTracing/VRayTracingStructs.hpp"

#include <array>

namespace vk {
struct PipelineShaderStageCreateInfo;
}
namespace VulkanCore::RTX {
enum ERayTracingStageIndices : int;
}
namespace VulkanUtils {
class VEffect;
}
namespace VulkanCore {
class VRayTracingShaders;
}
namespace VulkanCore {
class VDescriptorSetLayout;
}
namespace VulkanCore {
class VDevice;
namespace RTX {

class VRayTracingPipeline: public VObject {
public:
  VRayTracingPipeline(const VulkanCore::VDevice& device, const VulkanCore::VRayTracingShaders& rayTracingShaders,  const VulkanCore::VDescriptorSetLayout& descSetLayout);
  vk::RayTracingPipelineCreateInfoKHR Init();

private:
  void CreateCreatePipelineShaders();

  /**
   * Creates shader hit groups. Each entry in the Shader Binding Table (SBT)
   * corresponds to a shader group, which may include up to three shaders:
   *   - Closest Hit
   *   - Any Hit
   *   - Intersection
   *
   * Hit shaders are grouped under VK_SHADER_GROUP_TYPE_HIT_GROUP.
   * Miss, ray generation, and callable shaders use VK_SHADER_GROUP_TYPE_GENERAL.
   *
   * Examples:
   *   - Triangle geometry:
   *       - Closest hit only
   *       - Closest hit + any hit
   *   - Procedural geometry:
   *       - Intersection + closest hit
   *       - Intersection + closest hit + any hit
   */
  void CreateShaderHitGroups();

  void CreatePipelineLayout();
private:
  //=======================
  // passed from costructor
  const VulkanCore::VDevice& m_device;
  const VulkanCore::VRayTracingShaders& m_rayTracingShaders;
  const VulkanCore::VDescriptorSetLayout& m_descSetLayout;

  //=======================
  // pipeline create info
  vk::RayTracingPipelineCreateInfoKHR m_rtxPipelineCreateInfo;

  //=======================
  // pipeline properties
  std::array<vk::PipelineShaderStageCreateInfo, ERayTracingStageIndices::ShaderGroupCount> m_shaderStages;

  //========================
  // shader groups
  std::vector<vk::RayTracingShaderGroupCreateInfoKHR> m_shaderGroups;

  //========================
  // pipeline layout
  vk::PipelineLayout m_pipelineLayout;


private:
  friend class VulkanUtils::VEffect;
};

} // RTX
} // VulkanCore

#endif //VRAYTRACINGPIPELINE_HPP
