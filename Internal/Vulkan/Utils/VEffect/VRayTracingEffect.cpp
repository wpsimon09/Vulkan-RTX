//
// Created by wpsimon09 on 22/04/25.
//

#include "VRayTracingEffect.hpp"

#include "Vulkan/Utils/VResrouceGroup/VResrouceGroup.hpp"
#include "Vulkan/VulkanCore/Pipeline/VRayTracingPipeline.hpp"

namespace VulkanUtils {
VRayTracingEffect::VRayTracingEffect(const VulkanCore::VDevice&                          device,
                                     const VulkanCore::RTX::RTXShaderPaths&              shaderPaths,
                                     const std::string&                                  name,
                                     std::shared_ptr<VulkanUtils::VShaderResrouceGroup>& descriptorSet)
    : VEffect(device, name, descriptorSet)
    , m_shaders(device, shaderPaths)
{
    m_rtPipeline =
        std::make_unique<VulkanCore::RTX::VRayTracingPipeline>(device, m_shaders, descriptorSet->GetDescriptorSetLayout());
    m_rtPipeline->Init();
}
void VRayTracingEffect::BuildEffect()
{
    auto pipeline = m_device.GetDevice().createRayTracingPipelineKHR(nullptr, nullptr, m_rtPipeline->m_rtxPipelineCreateInfo,
                                                                      nullptr, m_device.DispatchLoader);
    VulkanUtils::Check(pipeline.result);

    m_rtPipeline->m_rtPipelineHandle = pipeline.value;

    m_rtPipeline->CreateShaderBindingTable();
    m_shaders.DestroyShaderModules();
}

vk::PipelineLayout VRayTracingEffect::GetPipelineLayout()
{
    return m_rtPipeline->GetPipelineLayout();
}

void VRayTracingEffect::BindPipeline(const vk::CommandBuffer& cmdBuffer) {}
void VRayTracingEffect::Destroy() {}
}  // namespace VulkanUtils