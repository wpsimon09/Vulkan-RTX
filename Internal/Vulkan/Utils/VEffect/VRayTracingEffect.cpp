//
// Created by wpsimon09 on 22/04/25.
//

#include "VRayTracingEffect.hpp"

#include "Vulkan/VulkanCore/Pipeline/VRayTracingPipeline.hpp"

namespace VulkanUtils {
VRayTracingEffect::VRayTracingEffect(const VulkanCore::VDevice&                          device,
                                     const VulkanCore::RTX::RTXShaderPaths&              shaderPaths,
                                     const std::string&                                  name,
                                     VulkanCore::VDescriptorLayoutCache& descLayoutCache)
    : VEffect(device, name, descLayoutCache ,EShaderBindingGroup::RayTracing)
    , m_shaders(device, shaderPaths)
{
    CreateLayouts(m_shaders.GetReflectionData());
    m_rtPipeline =
        std::make_unique<VulkanCore::RTX::VRayTracingPipeline>(device, m_shaders, m_descriptorSetLayouts);
    m_rtPipeline->Init();

}
void VRayTracingEffect::BuildEffect()
{
    auto pipeline = m_device.GetDevice().createRayTracingPipelineKHR(nullptr, nullptr, m_rtPipeline->m_rtxPipelineCreateInfo,
                                                                      nullptr, m_device.DispatchLoader);
    VulkanUtils::Check(pipeline.result);

    m_rtPipeline->m_rtPipelineHandle = pipeline.value;

    m_rtPipeline->CreateShaderBindingTable();

   //m_shaders.DestroyShaderModules();
}

vk::PipelineLayout VRayTracingEffect::GetPipelineLayout()
{
    return m_rtPipeline->GetPipelineLayout();
}
VulkanCore::RTX::VRayTracingPipeline& VRayTracingEffect::GetRTXPipeline() {
    return *m_rtPipeline;
}

void VRayTracingEffect::BindPipeline(const vk::CommandBuffer& cmdBuffer) {
    cmdBuffer.bindPipeline(vk::PipelineBindPoint::eRayTracingKHR, m_rtPipeline->m_rtPipelineHandle, m_device.DispatchLoader);   
}
void VRayTracingEffect::Destroy()
{
   m_rtPipeline->Destroy();
}
void VRayTracingEffect::BindDescriptorSet(const vk::CommandBuffer& cmdBuffer, uint32_t frame, uint32_t set) {}

}  // namespace VulkanUtils