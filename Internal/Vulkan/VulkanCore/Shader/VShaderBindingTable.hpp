//
// Created by wpsimon09 on 21/04/25.
//

#ifndef VSHADERBINDINGTABLE_HPP
#define VSHADERBINDINGTABLE_HPP
#include "Vulkan/VulkanCore/RayTracing/VRayTracingStructs.hpp"


#include <filesystem>


namespace VulkanCore {
class VDevice;

class VShaderBindingTable
{
  public:
    VShaderBindingTable(const VulkanCore::VDevice& device, RTX::RTXShaderPaths& shaders);

  private:
    void CreateShaderModules(RTX::RTXShaderPaths& shaders);

  private:
    const VulkanCore::VDevice& m_device;
    vk::ShaderModule m_rayGenShader;
    vk::ShaderModule m_missShader;
    vk::ShaderModule m_hitShader;
    std::array<vk::PipelineShaderStageCreateInfo, RTX::ERayTracingStageIndices::ShaderGroupCount> m_shaderStages;

};

}  // namespace VulkanCore

#endif  //VSHADERBINDINGTABLE_HPP
