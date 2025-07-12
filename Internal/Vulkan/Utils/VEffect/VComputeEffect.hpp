#ifndef VCOMPUTEEFFECT_HPP
#define VCOMPUTEEFFECT_HPP
#include "VEffect.hpp"


#include <variant>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_handles.hpp>

#include "Vulkan/VulkanCore/Descriptors/VDescriptorAllocator.hpp"
#include "Vulkan/VulkanCore/Shader/VShader.hpp"

namespace VulkanUtils {
class VComputeEffect : public VulkanUtils::VEffect
{
  public:
    VComputeEffect(const VulkanCore::VDevice&          device,
                   const std::string&                  name,
                   const std::string&                  computeShaderPath,
                   VulkanCore::VDescriptorLayoutCache& descLayoutCache,
                   EShaderBindingGroup                 bindingGroup = EShaderBindingGroup::ForwardUnlit);

    void               BuildEffect() override;
    vk::PipelineLayout GetPipelineLayout() override;
    void               BindPipeline(const vk::CommandBuffer& cmdBuffer) override;
    void               Destroy() override;
    void               BindDescriptorSet(const vk::CommandBuffer& cmdBuffer, uint32_t frame, uint32_t set) override;

  private:
    // since compute pipeline is much much simpler to set up i will not ceate dedicated calss for it
    vk::Pipeline        m_computePipeline;
    VulkanCore::VShader m_computeShader;
};
}  // namespace VulkanUtils

#endif