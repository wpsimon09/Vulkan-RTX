//
// Created by wpsimon09 on 21/04/25.
//

#include "VRayTracingShaders.hpp"

#include "VShader.hpp"
#include "Vulkan/Utils/VGeneralUtils.hpp"

namespace VulkanCore {
VRayTracingShaders::VRayTracingShaders(const VulkanCore::VDevice& device, const RTX::RTXShaderPaths& shaders)
    : m_device(device)
{
    CreateShaderModules(shaders);
}
 vk::ShaderModule VRayTracingShaders::GetShaderModule(VulkanCore::RTX::ERayTracingStageIndices shaderType)
{
    if(m_shaderModules.contains(shaderType))
    {
        return m_shaderModules[shaderType];
    }
    throw std::runtime_error("Shader type not found, make sure you are creating the correct shaders ! ");
}
const ReflectionData& VRayTracingShaders::GetReflectionData() {return m_reflectionData;}

void VRayTracingShaders::DestroyShaderModules()
{
    for(auto& s : m_shaderModules)
    {
        m_device.GetDevice().destroyShaderModule(s.second);
    }
    m_shaderModules.clear();
}

void VRayTracingShaders::CreateShaderModules(const RTX::RTXShaderPaths& shaders)
{
    Utils::Logger::LogInfoVerboseOnly("Creating ray tracing shader modules");

    auto rayGenSPIRV        = VulkanUtils::ReadSPIRVShader(shaders.rayGenPath);
    auto rayMissSPIRV       = VulkanUtils::ReadSPIRVShader(shaders.missPath);
    auto rayMissShadowSPIRV = VulkanUtils::ReadSPIRVShader(shaders.missShadowPath);
    auto rayHitSPIRV        = VulkanUtils::ReadSPIRVShader(shaders.rayHitPath);

    m_shaderModules[RTX::ERayTracingStageIndices::RayGen] = VulkanUtils::CreateShaderModule(m_device, rayGenSPIRV);
    m_shaderModules[RTX::ERayTracingStageIndices::Miss]   = VulkanUtils::CreateShaderModule(m_device, rayMissSPIRV);
    m_shaderModules[RTX::ERayTracingStageIndices::MissShadow] = VulkanUtils::CreateShaderModule(m_device, rayMissShadowSPIRV);
    m_shaderModules[RTX::ERayTracingStageIndices::ClosestHit] = VulkanUtils::CreateShaderModule(m_device, rayHitSPIRV);

    m_reflectionData.AddShader(rayGenSPIRV.data(), rayGenSPIRV.size()*sizeof(char), vk::ShaderStageFlagBits::eRaygenKHR);
    m_reflectionData.AddShader(rayMissSPIRV.data(), rayMissSPIRV.size()*sizeof(char), vk::ShaderStageFlagBits::eMissKHR);
    m_reflectionData.AddShader(rayMissShadowSPIRV.data(), rayMissShadowSPIRV.size()*sizeof(char), vk::ShaderStageFlagBits::eMissKHR);
    m_reflectionData.AddShader(rayHitSPIRV.data(), rayHitSPIRV.size()*sizeof(char), vk::ShaderStageFlagBits::eClosestHitKHR);

    Utils::Logger::LogInfoVerboseOnly("Created Ray tracing shader module");

    Utils::Logger::LogSuccess("Shader modules created !");
}
}  // namespace VulkanCore