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
void VRayTracingShaders::ReOrderBinding() {
    #warning "Using of this function will forcefully overwrited whatever bindings were reflected by SPRIV to the hardcoded ones here";

    ReflectionData newReflectionData;
    ReflecSetLayoutData newLayoutData;

    newLayoutData.bindings.push_back({0, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eAll});
    newLayoutData.bindings.push_back({1, vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eAll});
    newLayoutData.bindings.push_back({2, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eAll});
    newLayoutData.bindings.push_back({3, vk::DescriptorType::eAccelerationStructureKHR, 1, vk::ShaderStageFlagBits::eAll});
    newLayoutData.bindings.push_back({4, vk::DescriptorType::eStorageImage, 1, vk::ShaderStageFlagBits::eAll});
    newLayoutData.bindings.push_back({5, vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eAll});
    newLayoutData.bindings.push_back({6, vk::DescriptorType::eStorageImage, 1, vk::ShaderStageFlagBits::eAll});


    newLayoutData.variableNames.push_back({"_globalData", vk::DescriptorType::eUniformBuffer});
    newLayoutData.variableNames.push_back({"_lightInfo", vk::DescriptorType::eUniformBuffer});
    newLayoutData.variableNames.push_back({"_objectInfo", vk::DescriptorType::eUniformBuffer});
    newLayoutData.variableNames.push_back({"_tlas", vk::DescriptorType::eUniformBuffer});
    newLayoutData.variableNames.push_back({"_resultImage", vk::DescriptorType::eUniformBuffer});
    newLayoutData.variableNames.push_back({"_materialInfo", vk::DescriptorType::eUniformBuffer});
    newLayoutData.variableNames.push_back({"_accumulationimage", vk::DescriptorType::eUniformBuffer});

    newLayoutData.setNumber = 0;

    newLayoutData.createInfo.bindingCount = newLayoutData.bindings.size();
    newLayoutData.createInfo.pBindings = newLayoutData.bindings.data();
    newLayoutData.createInfo.flags = {};

    newReflectionData.descriptorSets[0] = std::move(newLayoutData);

    m_reflectionData = std::move(newReflectionData);;

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