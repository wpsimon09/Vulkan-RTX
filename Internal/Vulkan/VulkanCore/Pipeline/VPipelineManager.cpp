//
// Created by wpsimon09 on 06/10/24.
//

#include "VPipelineManager.hpp"

#include "VGraphicsPipeline.hpp"
#include "Application/Logger/Logger.hpp"
#include "Vulkan/VulkanCore/Descriptors/VDescriptorSetLayout.hpp"
#include "Vulkan/VulkanCore/Shader/VShader.hpp"
#include "Vulkan/VulkanCore/SwapChain/VSwapChain.hpp"


VulkanCore::VPipelineManager::VPipelineManager(const VulkanCore::VDevice &device,
                                               const VulkanCore::VSwapChain &swapChain,
                                               const Renderer::RenderTarget &renderTarget,
                                               const VulkanUtils::VPushDescriptorManager& pushDescriptorManager) :
    m_device(device), m_swapChain(swapChain), m_renderTarget(renderTarget), m_pushDescriptorSetManager(pushDescriptorManager) {
}

void VulkanCore::VPipelineManager::DestroyPipelines() {
    for (auto &pipeline : m_pipelines) {
        pipeline.second->Destroy();
        m_device.GetDevice().destroyPipeline(pipeline.second->GetPipelineInstance());
    }
}


void VulkanCore::VPipelineManager::InstantiatePipelines() {

    GeneratePipelines();
    Utils::Logger::LogInfoVerboseOnly("Creating " + std::to_string(m_pipelines.size()) + " graphics pipelines...");

    std::vector<vk::GraphicsPipelineCreateInfo> graphicsPipelineCreateInfos;
    for (auto &pipeline : m_pipelines) {
        graphicsPipelineCreateInfos.emplace_back(pipeline.second->GetGraphicsPipelineCreateInfoStruct());
    }

    auto createdVkPipelines = m_device.GetDevice().createGraphicsPipelines(nullptr, graphicsPipelineCreateInfos);

    assert(createdVkPipelines.value.size() == m_pipelines.size() && createdVkPipelines.result == vk::Result::eSuccess && "Failed to create pipelines !");

    Utils::Logger::LogSuccess("Successfully created " + std::to_string(m_pipelines.size()) + " graphics pipelines");

    Utils::Logger::LogInfoVerboseOnly("Binding pipelines...");
    int i = 0;
    for (auto &pipeline : m_pipelines) {

        assert(createdVkPipelines.result == vk::Result::eSuccess);
        if (createdVkPipelines.result == vk::Result::ePipelineCompileRequired) {
            throw std::runtime_error("Pipelinpe at index " + std::to_string(i) + " needs to be compiled");
        }

        pipeline.second->SetCreatedPipeline(createdVkPipelines.value[i]);
        i++;
    }
    Utils::Logger::LogSuccess("Successfully bound " + std::to_string(i) + " graphics pipelines");

    m_baseShader->DestroyExistingShaderModules();
    m_rtxShader->DestroyExistingShaderModules();
    m_debugLinesShader->DestroyExistingShaderModules();
    m_outlineShader->DestroyExistingShaderModules();
    m_multiLightShader->DestroyExistingShaderModules();
    m_editorBilboardShader->DestroyExistingShaderModules();
    m_debugGeometryShader->DestroyExistingShaderModules();
}

const VulkanCore::VGraphicsPipeline &VulkanCore::VPipelineManager::GetPipeline(EPipelineType pipeline) const {
    auto foundPipeline = m_pipelines.find(pipeline);
    assert(foundPipeline != m_pipelines.end());
    return *foundPipeline->second;
}

const std::vector<std::reference_wrapper<const VulkanCore::VGraphicsPipeline>>
VulkanCore::VPipelineManager::GetAllPipelines() const {
    std::vector<std::reference_wrapper<const VGraphicsPipeline>> pipelines;
    pipelines.reserve(m_pipelines.size());
    for (auto &pipeline : m_pipelines) {
        pipelines.emplace_back(std::ref(*pipeline.second));
    }
    return pipelines;

}

void VulkanCore::VPipelineManager::GeneratePipelines() {

    //==================================
    // MAIN SCENE PIPELINE
    //==================================
    auto basicPipelineShaderVertexSource = "Shaders/Compiled/BasicTriangle.vert.slang.spv";
    auto basicPipelineFragmentShaderSource = "Shaders/Compiled/GGXColourFragmentMultiLight.frag.slang.spv";
    m_baseShader = std::make_unique<VShader>(m_device, basicPipelineShaderVertexSource,
        basicPipelineFragmentShaderSource);

    auto pipeline = std::make_unique<VGraphicsPipeline>(m_device, m_swapChain, *m_baseShader, m_renderTarget, m_pushDescriptorSetManager.GetLayout());
    pipeline->Init();
    pipeline->SetPipelineType(EPipelineType::RasterPBRTextured);
    pipeline->SetPrimitiveTopology(vk::PrimitiveTopology::eTriangleList);
    m_pipelines[EPipelineType::RasterPBRTextured] = std::move(pipeline);


    //==================================
    // MULTI LIGHT PIPELINE
    //==================================
    auto multiLightPipelineVertexShaderSource = "Shaders/Compiled/BasicTriangle.vert.slang.spv";
    auto multiLightPipelineFragmnetShaderSource = "Shaders/Compiled/GGXColourFragmentMultiLight.frag.slang.spv";
    m_multiLightShader = std::make_unique<VShader>(m_device, multiLightPipelineVertexShaderSource,
        multiLightPipelineFragmnetShaderSource);

    pipeline = std::make_unique<VGraphicsPipeline>(m_device, m_swapChain, *m_multiLightShader, m_renderTarget, m_pushDescriptorSetManager.GetLayout());
    pipeline->Init();
    pipeline->SetPipelineType(EPipelineType::MultiLight);
    pipeline->SetPrimitiveTopology(vk::PrimitiveTopology::eTriangleList);
    m_pipelines[EPipelineType::MultiLight] = std::move(pipeline);

    

    //==================================
    // TRANSPARENT PASS PIPELINE
    //==================================
    pipeline = std::make_unique<VGraphicsPipeline>(m_device, m_swapChain, *m_multiLightShader, m_renderTarget, m_pushDescriptorSetManager.GetLayout());
    pipeline->Init();
    pipeline->SetPipelineType(EPipelineType::Transparent);
    pipeline->EnableBlendingAdditive();
    pipeline->SetPrimitiveTopology(vk::PrimitiveTopology::eTriangleList);
    m_pipelines[EPipelineType::Transparent] = std::move(pipeline);


    //==================================
    // EDITOR BILLBOARD PIPELINE
    //==================================
    auto editorBillboardVertexShaderSource = "Shaders/Compiled/EditorBillboard.vert.slang.spv";
    auto editorBillboardFragmentShaderSource = "Shaders/Compiled/EditorBilboard.frag.slang.spv";
    m_editorBilboardShader = std::make_unique<VShader>(m_device, editorBillboardVertexShaderSource,
        editorBillboardFragmentShaderSource);

    pipeline = std::make_unique<VGraphicsPipeline>(m_device, m_swapChain, *m_editorBilboardShader, m_renderTarget, m_pushDescriptorSetManager.GetLayout());
    pipeline->Init();

    pipeline->SetPipelineType(EPipelineType::EditorBillboard);
    pipeline->SetPrimitiveTopology(vk::PrimitiveTopology::eTriangleList);
    pipeline->SetCullMode(vk::CullModeFlagBits::eNone);
    pipeline->DisableDepthTest();
    m_pipelines[EPipelineType::EditorBillboard] = std::move(pipeline);

    //==================================
    // RAY TRACING, ARTIFICIAL PIPELINE
    //==================================
    auto rtxVertexShaderPath = "Shaders/Compiled/RayTracer.vert.slang.spv";
    auto rtxFragmentShaderPath = "Shaders/Compiled/RayTracer.frag.slang.spv";
    m_rtxShader = std::make_unique<VShader>(m_device,rtxVertexShaderPath,
                                             rtxFragmentShaderPath);
    pipeline = std::make_unique<VGraphicsPipeline>(m_device, m_swapChain, *m_rtxShader, m_renderTarget, m_pushDescriptorSetManager.GetLayout());
    pipeline->Init();
    pipeline->SetPipelineType(EPipelineType::RTX);
    pipeline->SetPrimitiveTopology(vk::PrimitiveTopology::eTriangleList);
    pipeline->SetCullMode(vk::CullModeFlagBits::eNone);
    m_pipelines[EPipelineType::RTX] = std::move(pipeline);

    //==================================
    // DEBUG LINE FRAGMENT
    //==================================
    auto debugLineVertex = "Shaders/Compiled/BasicTriangle.vert.slang.spv";
    auto debugLineFragment = "Shaders/Compiled/DebugLines.frag.slang.spv";
    m_debugLinesShader = std::make_unique<VShader>(m_device,debugLineVertex,
                                             debugLineFragment);

    pipeline = std::make_unique<VGraphicsPipeline>(m_device, m_swapChain, *m_debugLinesShader, m_renderTarget, m_pushDescriptorSetManager.GetLayout());
    pipeline->Init();
    pipeline->SetPipelineType(EPipelineType::DebugLines);
    pipeline->SetPrimitiveTopology(vk::PrimitiveTopology::eTriangleList);
    pipeline->SetCullMode(vk::CullModeFlagBits::eNone);
    pipeline->SetPolygonMode(vk::PolygonMode::eLine);
    pipeline->SetLineWidth(2);

    m_pipelines[EPipelineType::DebugLines] = std::move(pipeline);


    //==================================
    // OUTLINE PIPELINE
    //==================================
    auto outlineVertex = "Shaders/Compiled/BasicTriangle.vert.slang.spv";
    auto outlineFragment = "Shaders/Compiled/Outliines.frag.slang.spv";
    m_outlineShader = std::make_unique<VShader>(m_device,outlineVertex,
                                             outlineFragment);

    pipeline = std::make_unique<VGraphicsPipeline>(m_device, m_swapChain, *m_outlineShader, m_renderTarget, m_pushDescriptorSetManager.GetLayout());
    pipeline->Init();
    pipeline->SetPipelineType(EPipelineType::Outline);
    pipeline->SetPrimitiveTopology(vk::PrimitiveTopology::eTriangleList);
    pipeline->SetCullMode(vk::CullModeFlagBits::eFront);
    pipeline->SetLineWidth(7.0f);
    pipeline->SetPolygonMode(vk::PolygonMode::eLine);
    pipeline->DisableDepthWrite();
    //pipeline->DisableDepthTest();
    m_pipelines[EPipelineType::Outline] = std::move(pipeline);

    //==================================
    // DEBUG SHAPES PIPELINE
    // - same as outline but with depth test disabled
    //==================================
    auto debugGeometryVertex = "Shaders/Compiled/BasicTriangle.vert.slang.spv";
    auto debugGeometryFragment = "Shaders/Compiled/DebugGeometry.frag.slang.spv";
    m_debugGeometryShader = std::make_unique<VShader>(m_device,debugGeometryVertex, debugGeometryFragment);

    pipeline = std::make_unique<VGraphicsPipeline>(m_device, m_swapChain, *m_debugGeometryShader, m_renderTarget, m_pushDescriptorSetManager.GetLayout());
    pipeline->Init();
    pipeline->SetPipelineType(EPipelineType::DebugShadpes);
    pipeline->SetPrimitiveTopology(vk::PrimitiveTopology::eTriangleList);
    pipeline->SetCullMode(vk::CullModeFlagBits::eNone);
    pipeline->SetLineWidth(2.0f);
    pipeline->SetPolygonMode(vk::PolygonMode::eLine);
    m_pipelines[EPipelineType::DebugShadpes] = std::move(pipeline);

}




