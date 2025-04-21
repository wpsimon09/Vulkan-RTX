//
// Created by wpsimon09 on 21/04/25.
//

#ifndef VSHADERBINDINGTABLE_HPP
#define VSHADERBINDINGTABLE_HPP
#include "Vulkan/VulkanCore/RayTracing/VRayTracingStructs.hpp"


#include <filesystem>


namespace VulkanCore {
class VDevice;

class VRayTracingShaders
{
  public:
    VRayTracingShaders(const VulkanCore::VDevice& device, RTX::RTXShaderPaths& shaders);

    const vk::ShaderModule& GetShaderModule(VulkanCore::RTX::ERayTracingStageIndices shaderType) const ;

  private:
    void CreateShaderModules(RTX::RTXShaderPaths& shaders);
  private:
    const VulkanCore::VDevice& m_device;
    std::unordered_map<RTX::ERayTracingStageIndices, VkShaderModule> m_shaderModules;


};

}  // namespace VulkanCore

#endif  //VSHADERBINDINGTABLE_HPP
