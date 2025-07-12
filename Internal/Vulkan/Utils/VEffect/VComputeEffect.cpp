#include "VComputeEffect.hpp"

namespace VulkanUtils {
VComputeEffect::VComputeEffect(const VulkanCore::VDevice&          device,
                               const std::string&                  name,
                               const std::string&                  computeShaderPath,
                               VulkanCore::VDescriptorLayoutCache& descLayoutCache,
                               EShaderBindingGroup                 bindingGroup)
    : VEffect(device, name, descLayoutCache, bindingGroup)
    , m_computeShader(device, computeShaderPath)
{
}

void VComputeEffect::BuildEffect() {}

vk::PipelineLayout VComputeEffect::GetPipelineLayout() {}

void VComputeEffect::BindPipeline(const vk::CommandBuffer& cmdBuffer) {}

void VComputeEffect::Destroy() {}

void VComputeEffect::BindDescriptorSet(const vk::CommandBuffer& cmdBuffer, uint32_t frame, uint32_t set) {}


}  // namespace VulkanUtils