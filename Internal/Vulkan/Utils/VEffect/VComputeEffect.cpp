#include "VComputeEffect.hpp"
#include "Application/Logger/Logger.hpp"
#include "Vulkan/Global/GlobalVariables.hpp"
#include "Vulkan/Utils/VGeneralUtils.hpp"
#include <vulkan/vulkan_enums.hpp>
#include <vulkan/vulkan_structs.hpp>

namespace VulkanUtils {
VComputeEffect::VComputeEffect(const VulkanCore::VDevice&          device,
                               const std::string&                  name,
                               const std::string&                  computeShaderPath,
                               VulkanCore::VDescriptorLayoutCache& descLayoutCache,
                               EShaderBindingGroup                 bindingGroup)
    : VEffect(device, name, descLayoutCache, bindingGroup)
    , m_computeShader(device, computeShaderPath)
{
    CreateLayouts(m_computeShader.GetReflectionData());
}

void VComputeEffect::BuildEffect()
{
    vk::PipelineShaderStageCreateInfo computeShaderCI;
    computeShaderCI.stage  = vk::ShaderStageFlagBits::eCompute;
    computeShaderCI.module = m_computeShader.GetShaderModule(GlobalVariables::SHADER_TYPE::COMPUTE);
    computeShaderCI.pName  = "main";
    computeShaderCI.flags  = {};

    vk::PipelineLayoutCreateInfo pipelineLayoutCi;
    pipelineLayoutCi.pSetLayouts    = m_descriptorSetLayouts.data();
    pipelineLayoutCi.setLayoutCount = m_descriptorSetLayouts.size();

    pipelineLayoutCi.pushConstantRangeCount = m_computeShader.GetReflectionData().PCs.size();
    pipelineLayoutCi.pPushConstantRanges    = m_computeShader.GetReflectionData().PCs.data();

    VulkanUtils::Check(m_device.GetDevice().createPipelineLayout(&pipelineLayoutCi, nullptr, &m_computePipelienLayout),
                       vk::Result::eSuccess);

    vk::ComputePipelineCreateInfo computePipelineCI;
    computePipelineCI.layout = m_computePipelienLayout;
    computePipelineCI.stage  = computeShaderCI;

    auto result_pipeline = m_device.GetDevice().createComputePipeline({}, *computePipelineCI);
    VulkanUtils::Check(result_pipeline.result, vk::Result::eSuccess);
    m_computePipeline = result_pipeline.value;
    m_computeShader.DestroyExistingShaderModules();

    Utils::Logger::LogSuccess("Compute pipeline for effect: " + m_name + " created !");
}

vk::PipelineLayout VComputeEffect::GetPipelineLayout()
{
    return m_computePipelienLayout;
}

void VComputeEffect::BindPipeline(const vk::CommandBuffer& cmdBuffer)
{
    cmdBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, m_computePipeline);
}

void VComputeEffect::Destroy()
{
    m_device.GetDevice().destroyPipeline(m_computePipeline);
}

void VComputeEffect::BindDescriptorSet(const vk::CommandBuffer& cmdBuffer, uint32_t frame, uint32_t set)
{
    for(int i = 0; i < m_descriptorSets.size(); i++)
    {
        cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, m_computePipelienLayout, i, 1,
                                     &m_descriptorSets[i].sets[frame], 0, nullptr);
    }
}
}  // namespace VulkanUtils
