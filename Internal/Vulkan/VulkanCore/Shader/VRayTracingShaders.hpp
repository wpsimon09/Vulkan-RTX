//
// Created by wpsimon09 on 21/04/25.
//

#ifndef VSHADERBINDINGTABLE_HPP
#define VSHADERBINDINGTABLE_HPP
#include "VShader.hpp"
#include "Vulkan/VulkanCore/RayTracing/VRayTracingStructs.hpp"


#include <filesystem>


namespace VulkanCore {
class VDevice;

class VRayTracingShaders
{
  public:
    VRayTracingShaders(const VulkanCore::VDevice& device,const RTX::RTXShaderPaths& shaders);

    vk::ShaderModule GetShaderModule(VulkanCore::RTX::ERayTracingStageIndices shaderType)  ;
    const ReflectionData& GetReflectionData();

    void DestroyShaderModules();
    /**
     * This method will forcfully
     */
    void ReOrderBinding();
  private:
    void CreateShaderModules(const RTX::RTXShaderPaths& shaders);
  private:
    const VulkanCore::VDevice& m_device;
    ReflectionData m_reflectionData;

    std::unordered_map<RTX::ERayTracingStageIndices, VkShaderModule> m_shaderModules;
    std::unordered_map<RTX::ERayTracingStageIndices, ReflectionData> m_reflectionModeules;

    // used only in the case of ray tracing until I figure out better alternative !
    std::vector<vk::DescriptorSetLayoutBinding> m_overWrittenDescriptorSetLayoutBindings;

};

}  // namespace VulkanCore

#endif  //VSHADERBINDINGTABLE_HPP
