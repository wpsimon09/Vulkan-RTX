//
// Created by wpsimon09 on 21/04/25.
//

#include "VRayTracingShaders.hpp"

#include "Vulkan/Utils/VGeneralUtils.hpp"

namespace VulkanCore {
VRayTracingShaders::VRayTracingShaders(const VulkanCore::VDevice& device, const RTX::RTXShaderPaths& shaders)
    : m_device(device)
{
    CreateShaderModules(shaders);
}
const vk::ShaderModule& VRayTracingShaders::GetShaderModule(VulkanCore::RTX::ERayTracingStageIndices shaderType) const
{
    if(m_shaderModules.contains(shaderType))
    {
        return m_shaderModules.at(shaderType);
    }
    throw std::runtime_error("Shader type not found, make sure you are creating the correct shaders ! ");
}

void VRayTracingShaders::DestroyShaderModules() {
    for (auto& s : m_shaderModules) {
        m_device.GetDevice().destroyShaderModule(s.second);
    }
    m_shaderModules.clear();
}

void VRayTracingShaders::CreateShaderModules(const RTX::RTXShaderPaths& shaders)
{
    Utils::Logger::LogInfoVerboseOnly("Creating ray tracing shader modules");

    auto rayGenSPIRV  = VulkanUtils::ReadSPIRVShader(shaders.rayGenPath);
    auto rayMissSPIRV = VulkanUtils::ReadSPIRVShader(shaders.missPath);
    auto rayHitSPIRV  = VulkanUtils::ReadSPIRVShader(shaders.rayHitPath);

    m_shaderModules[RTX::ERayTracingStageIndices::RayGen]     = VulkanUtils::CreateShaderModule(m_device, rayGenSPIRV);
    m_shaderModules[RTX::ERayTracingStageIndices::ClosestHit] = VulkanUtils::CreateShaderModule(m_device, rayHitSPIRV);
    m_shaderModules[RTX::ERayTracingStageIndices::Miss]       = VulkanUtils::CreateShaderModule(m_device, rayMissSPIRV);

    Utils::Logger::LogInfoVerboseOnly("Created Fragment shader module");

    Utils::Logger::LogSuccess("Shader modules created !");
}
}  // namespace VulkanCore