//
// Created by wpsimon09 on 19/03/25.
//

#include "VRasterEffect.hpp"
#include "Vulkan/VulkanCore/Shader/VShader.hpp"
#include "Vulkan/Renderer/RenderTarget/RenderTarget.hpp"
#include "Vulkan/VulkanCore/Pipeline/VGraphicsPipeline.hpp"

namespace VulkanUtils {
VRasterEffect::VRasterEffect(const VulkanCore::VDevice&                          device,
                             const std::string&                                  name,
                             const VulkanCore::VShader&                          shader,
                             VulkanCore::VDescriptorLayoutCache& descLayoutCache,
                             EShaderBindingGroup bindingGroup)
    : VEffect(device, name, descLayoutCache, bindingGroup)
{
    CreateLayouts(shader.GetReflectionData());


    m_pipeline = std::make_unique<VulkanCore::VGraphicsPipeline>(device, shader, m_descriptorSetLayouts);
    m_pipeline->Init();

}

VRasterEffect::VRasterEffect(const VulkanCore::VDevice&                          device,
                             const std::string&                                  name,
                             const std::string&                                  vertex,
                             const std::string&                                  fragment,
                             VulkanCore::VDescriptorLayoutCache& descLayoutCache,
                             EShaderBindingGroup bindingGroup)
    : VEffect(device, name, descLayoutCache, bindingGroup)
    , m_shader(std::in_place, device, vertex, fragment)
{
    if (m_shader.has_value())
        CreateLayouts(m_shader.value().GetReflectionData());
    else throw std::runtime_error("Failed to retrieve reflection data from compiled SPIRV-Shader");

    m_pipeline =
        std::make_unique<VulkanCore::VGraphicsPipeline>(device, m_shader.value(), m_descriptorSetLayouts);
    m_pipeline->Init();
}

VRasterEffect& VRasterEffect::SetDisableDepthTest()
{
    m_pipeline->m_depthStencil.depthTestEnable = false;
    return *this;
}

VRasterEffect& VRasterEffect::SetLineWidth(int lineWidth)
{
    m_pipeline->m_rasterizer.lineWidth = lineWidth;
    return *this;
}

VRasterEffect& VRasterEffect::SetCullFrontFace()
{
    m_pipeline->m_rasterizer.cullMode = vk::CullModeFlagBits::eFront;
    return *this;
}

VRasterEffect& VRasterEffect::SetCullNone()
{
    m_pipeline->SetCullMode(vk::CullModeFlagBits::eNone);
    return *this;
}

VRasterEffect& VRasterEffect::SetDisableDepthWrite()
{
    m_pipeline->m_depthStencil.depthWriteEnable = false;
    return *this;
}

VRasterEffect& VRasterEffect::SetTopology(vk::PrimitiveTopology topology)
{
    m_pipeline->m_inputAssembly.topology = topology;
    return *this;
}

VRasterEffect& VRasterEffect::SetPolygonLine()
{
    m_pipeline->m_rasterizer.polygonMode = vk::PolygonMode::eLine;
    return *this;
}

VRasterEffect& VRasterEffect::SetPolygonPoint()
{
    m_pipeline->m_rasterizer.polygonMode = vk::PolygonMode::ePoint;
    return *this;
}

VRasterEffect& VRasterEffect::EnableAdditiveBlending()
{
    m_pipeline->EnableBlendingAdditive();
    return *this;
}

VRasterEffect& VRasterEffect::OutputHDR()
{
    std::vector<vk::Format> formats = {vk::Format::eR16G16B16A16Sfloat};
    m_pipeline->m_outputFormats     = formats;
    return *this;
}

VRasterEffect& VRasterEffect::SetDepthOpEqual()
{
    m_pipeline->m_depthStencil.depthCompareOp = vk::CompareOp::eEqual;
    return *this;
}

VRasterEffect& VRasterEffect::SetDepthOpLessEqual()
{
    m_pipeline->m_depthStencil.depthCompareOp = vk::CompareOp::eLessOrEqual;
    return *this;
}

VRasterEffect& VRasterEffect::SetFrontFaceClockWise()
{
    m_pipeline->m_rasterizer.frontFace = vk::FrontFace::eClockwise;
    return *this;
}

VRasterEffect& VRasterEffect::SetVertexInputMode(EVertexInput inputMode)
{
    m_pipeline->CreateVertexInputBindingAndAttributes(inputMode);
    return *this;
}

VRasterEffect& VRasterEffect::SetStencilTestOutline()
{
    m_pipeline->m_depthStencil.back.compareOp   = vk::CompareOp::eNotEqual;
    m_pipeline->m_depthStencil.back.failOp      = vk::StencilOp::eKeep;
    m_pipeline->m_depthStencil.back.depthFailOp = vk::StencilOp::eKeep;
    m_pipeline->m_depthStencil.back.passOp      = vk::StencilOp::eReplace;
    m_pipeline->m_depthStencil.back.reference   = 1;


    m_pipeline->m_depthStencil.front = m_pipeline->m_depthStencil.back;

    return *this;
}

VRasterEffect& VRasterEffect::DisableStencil()
{
    m_pipeline->m_depthStencil.stencilTestEnable = vk::False;
    return *this;
}

VRasterEffect& VRasterEffect::SetDepthTestNever()
{
    m_pipeline->m_depthStencil.depthCompareOp = vk::CompareOp::eNever;
    return *this;
}

VRasterEffect& VRasterEffect::SetColourOutputFormat(vk::Format format)
{
    m_pipeline->SetColourOutputFormat(format);
    return *this;
}

VRasterEffect& VRasterEffect::SetPiplineNoMultiSampling()
{
    m_pipeline->m_multisampling.rasterizationSamples = vk::SampleCountFlagBits::e1;
    return *this;
}

VRasterEffect& VRasterEffect::SetNullVertexBinding()
{
    m_pipeline->m_vertexInputState.vertexAttributeDescriptionCount = 0;
    m_pipeline->m_vertexInputState.vertexBindingDescriptionCount   = 0;
    m_pipeline->m_vertexInputState.pVertexBindingDescriptions      = nullptr;
    m_pipeline->m_vertexInputState.pVertexAttributeDescriptions    = nullptr;
    return *this;
}

VRasterEffect& VRasterEffect::DissableFragmentWrite()
{
    m_pipeline->m_rasterizer.rasterizerDiscardEnable = vk::True;
    return *this;
}

VRasterEffect& VRasterEffect::SetDepthOpLess()
{
    m_pipeline->m_depthStencil.depthCompareOp = vk::CompareOp::eLess;
    return *this;
}

VRasterEffect& VRasterEffect::SetDepthOpAllways()
{
    m_pipeline->m_depthStencil.depthCompareOp = vk::CompareOp::eAlways;
    return *this;
}

void VRasterEffect::BuildEffect()
{

    auto pipelines = m_device.GetDevice().createGraphicsPipelines(nullptr, m_pipeline->GetGraphicsPipelineCreateInfoStruct());
    assert(pipelines.result == vk::Result::eSuccess);
    for(auto& p : pipelines.value)
    {
        m_pipeline->SetCreatedPipeline(p);
    }
    m_shader->DestroyExistingShaderModules();
}

vk::PipelineLayout VRasterEffect::GetPipelineLayout()
{
    return m_pipeline->GetPipelineLayout();
}

void VRasterEffect::BindPipeline(const vk::CommandBuffer& cmdBuffer)
{
    cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipeline->GetPipelineInstance());
}

void VRasterEffect::Destroy()
{
    m_pipeline->Destroy();
}
void VRasterEffect::BindDescriptorSet(const vk::CommandBuffer& cmdBuffer, uint32_t frame, uint32_t set) {
    cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_pipeline->GetPipelineLayout(), set, m_descriptorSets.size(), &m_descriptorSets[set].sets[frame], 0, nullptr);
}

}  // namespace VulkanUtils
