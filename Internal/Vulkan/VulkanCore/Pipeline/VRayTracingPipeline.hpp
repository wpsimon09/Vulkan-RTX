//
// Created by wpsimon09 on 21/04/25.
//

#ifndef VRAYTRACINGPIPELINE_HPP
#define VRAYTRACINGPIPELINE_HPP
#include "Vulkan/VulkanCore/VObject.hpp"
#include "Vulkan/VulkanCore/RayTracing/VRayTracingStructs.hpp"

#include <array>

namespace VulkanUtils {
class VRayTracingEffect;
}
namespace vk {
struct PipelineShaderStageCreateInfo;
}

namespace VulkanUtils {
class VRasterEffect;
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

class VRayTracingPipeline : public VObject
{
  public:
    VRayTracingPipeline(const VulkanCore::VDevice&              device,
                        const VulkanCore::VRayTracingShaders&   rayTracingShaders,
                        const VulkanCore::VDescriptorSetLayout& descSetLayout);
    void               Init();
    vk::PipelineLayout GetPipelineLayout();



      /*
    Each entry in the SBT is a Shader Group.
    A Shader Group can logically include up to 3 shaders:
        - Any-Hit Shader (optional)
        - Closest-Hit Shader (optional)
        - Intersection Shader (optional)

    In practice, each group is stored as ONE record in the SBT,
    but conceptually each group "spans" these logical shader slots.

    SBT Regions:

    +----------------------------+-----------------------------+-----------------------------+-----------------------------+
    |         RayGen            |             Miss            |           Hit Group         |          Callable           |
    +----------------------------+-----------------------------+-----------------------------+-----------------------------+
    | RayGen Shader Group       | Miss Shader Group 0         | Hit Group 0                 | Callable Shader Group 0     |
    |                           | Miss Shader Group 1         |   ↳ Any-Hit Shader (opt)    | Callable Shader Group 1     |
    |                           |                             |   ↳ Closest-Hit Shader (opt)|                             |
    |                           |                             |   ↳ Intersection Shader(opt)|                             |
    |                           |                             | Hit Group 1                 |                             |
    |                           |                             |   ↳ ...                     |                             |
    +----------------------------+-----------------------------+-----------------------------+-----------------------------+

    Each Shader Group Record in the buffer contains:
        - Shader Identifier (from vkGetRayTracingShaderGroupHandlesKHR)
        - Optional Custom Data (like material, object ID, etc.)
        - Proper Alignment + Stride per Vulkan spec

    Actual memory layout = flat array of records in GPU buffer.
    This table = conceptual aid for understanding grouping.
    */
    void CreateShaderBindingTable();

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

    void DestroyShaderModules();

  private:
    //=======================
    // passed from costructor
    const VulkanCore::VDevice&              m_device;
    const VulkanCore::VRayTracingShaders&   m_rayTracingShaders;
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

    //========================
    // pipeline layout
    vk::Pipeline m_rtPipelineHandle;

    std::unique_ptr<VulkanCore::VBuffer> m_shaderBindingTable;

    vk::StridedDeviceAddressRegionKHR    m_rGenRegion;
    vk::StridedDeviceAddressRegionKHR    m_rMissRegion;
    vk::StridedDeviceAddressRegionKHR    m_rHitRegion;
    vk::StridedDeviceAddressRegionKHR    m_rCallRegion;


  private:
    friend class VulkanUtils::VRayTracingEffect;
};

}  // namespace RTX
}  // namespace VulkanCore

#endif  //VRAYTRACINGPIPELINE_HPP
