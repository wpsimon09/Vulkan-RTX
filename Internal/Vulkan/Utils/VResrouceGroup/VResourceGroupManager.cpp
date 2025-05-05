//
// Created by wpsimon09 on 08/11/24.
//

#include "VResourceGroupManager.hpp"

#include "VResrouceGroup.hpp"
#include "Application/Logger/Logger.hpp"
#include "Vulkan/Global/GlobalVariables.hpp"
#include "Vulkan/VulkanCore/Descriptors/VDescriptorSetLayout.hpp"
#include "Vulkan/VulkanCore/Device/VDevice.hpp"
#include "Vulkan/VulkanCore/Pipeline/VGraphicsPipeline.hpp"
#include "Vulkan/VulkanCore/Buffer/VBuffer.hpp"

namespace VulkanUtils {
VResourceGroupManager::VResourceGroupManager(const VulkanCore::VDevice& device)
    : m_device(device)
{

    VulkanUtils::Unlit unlitSingleTexture{};
    auto layout = std::make_unique<VulkanCore::VDescriptorSetLayout>(m_device, unlitSingleTexture);
    m_pushDescriptors[EDescriptorLayoutStruct::UnlitSingleTexture] =
        std::make_shared<VShaderResrouceGroup>(m_device, std::string("Unlit single texture"), std::move(layout));

    VulkanUtils::ForwardShadingDstSet forwardShading{};
    layout = std::make_unique<VulkanCore::VDescriptorSetLayout>(m_device, forwardShading);
    m_pushDescriptors[EDescriptorLayoutStruct::ForwardShading] =
        std::make_shared<VShaderResrouceGroup>(m_device, std::string("Forward shading dst set"), std::move(layout));

    VulkanUtils::BasicDescriptorSet basicDescriptorSet{};
    layout = std::make_unique<VulkanCore::VDescriptorSetLayout>(m_device, basicDescriptorSet);
    m_pushDescriptors[EDescriptorLayoutStruct::Basic] =
        std::make_shared<VShaderResrouceGroup>(m_device, std::string("Basic descriptor set"), std::move(layout));

    VulkanUtils::EmtpyDescriptorSet empty{};
    layout = std::make_unique<VulkanCore::VDescriptorSetLayout>(m_device, empty);
    m_pushDescriptors[EDescriptorLayoutStruct::Empty] =
        std::make_shared<VShaderResrouceGroup>(m_device, std::string("Basic descriptor set"), std::move(layout));

    VulkanUtils::RayTracingDescriptorSet rayTracingDescriptorSet{};
    layout = std::make_unique<VulkanCore::VDescriptorSetLayout>(m_device, rayTracingDescriptorSet);
    m_pushDescriptors[EDescriptorLayoutStruct::RayTracing] =
        std::make_shared<VShaderResrouceGroup>(m_device, std::string("Ray tracing descriptor set"), std::move(layout));

    VulkanUtils::PostProcessingDescriptorSet postProcessingDescriptorSet{};
    layout = std::make_unique<VulkanCore::VDescriptorSetLayout>(m_device, postProcessingDescriptorSet);
    m_pushDescriptors[EDescriptorLayoutStruct::PostProcessing] =
        std::make_shared<VShaderResrouceGroup>(m_device, std::string("Postprocessing descriptor set"), std::move(layout));
}


void VResourceGroupManager::Destroy()
{
    for(auto& pushDst : m_pushDescriptors)
    {
        pushDst.second->Destroy();
    }
}

}  // namespace VulkanUtils