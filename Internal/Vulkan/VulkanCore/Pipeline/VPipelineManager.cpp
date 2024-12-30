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
                                               const VulkanCore::VRenderPass &renderPass,
                                               const VulkanUtils::VPushDescriptorManager& pushDescriptorManager) :
    m_device(device), m_swapChain(swapChain), m_renderPass(renderPass), m_pushDescriptorSetManager(pushDescriptorManager) {
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

    assert(createdVkPipelines.value.size() == m_pipelines.size());
    Utils::Logger::LogSuccess("Successfully created " + std::to_string(m_pipelines.size()) + " graphics pipelines");

    Utils::Logger::LogInfoVerboseOnly("Binding pipelines...");
    int i = 0;
    for (auto &pipeline : m_pipelines) {

        assert(createdVkPipelines.result == vk::Result::eSuccess);
        if (createdVkPipelines.result == vk::Result::ePipelineCompileRequired) {
            throw std::runtime_error("Pipeline at index " + std::to_string(i) + " needs to be compiled");
        }

        pipeline.second->SetCreatedPipeline(createdVkPipelines.value[i]);
        i++;
    }
    Utils::Logger::LogSuccess("Successfully bound " + std::to_string(i) + " graphics pipelines");

    m_baseShader->DestroyExistingShaderModules();
    m_rtxShader->DestroyExistingShaderModules();
    m_debugLinesShader->DestroyExistingShaderModules();
}

const VulkanCore::VGraphicsPipeline &VulkanCore::VPipelineManager::GetPipeline(PIPELINE_TYPE pipeline) const {
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

void VulkanCore::VPipelineManager::GeneratePipelines()  {

    //==================================
    // MAIN SCENE PIPELINE
    //==================================
    auto basicPipelineShaderVertexSource = "Shaders/Compiled/BasicTriangle.vert.slang.spv";
    auto basicPipelineFragmentShaderSource = "Shaders/Compiled/GGXColourFragment.frag.slang.spv";
    m_baseShader = std::make_unique<VShader>(m_device, basicPipelineShaderVertexSource,
        basicPipelineFragmentShaderSource);

    auto pipeline = std::make_unique<VGraphicsPipeline>(m_device, m_swapChain, *m_baseShader, m_renderPass, m_pushDescriptorSetManager.GetLayout());
    pipeline->Init();
    pipeline->SetPipelineType(PIPELINE_TYPE_RASTER_PBR_TEXTURED);
    pipeline->SetPrimitiveTopology(vk::PrimitiveTopology::eTriangleList);
    m_pipelines[PIPELINE_TYPE_RASTER_PBR_TEXTURED] = std::move(pipeline);

    //==================================
    // RAY TRACING, ARTIFICIAL PIPELINE
    //==================================
    auto rtxVertexShaderPath = "Shaders/Compiled/RayTracer.vert.slang.spv";
    auto rtxFragmentShaderPath = "Shaders/Compiled/RayTracer.frag.slang.spv";
    m_rtxShader = std::make_unique<VShader>(m_device,rtxVertexShaderPath,
                                             rtxFragmentShaderPath);
    pipeline = std::make_unique<VGraphicsPipeline>(m_device, m_swapChain, *m_rtxShader, m_renderPass, m_pushDescriptorSetManager.GetLayout());
    pipeline->Init();
    pipeline->SetPipelineType(PIPELINE_TYPE_RTX);
    pipeline->SetPrimitiveTopology(vk::PrimitiveTopology::eTriangleList);
    m_pipelines[PIPELINE_TYPE_RTX] = std::move(pipeline);

    //==================================
    // DEBUG LINE FRAGMENT
    //==================================
    auto debugLineVertex = "Shaders/Compiled/BasicTriangle.vert.slang.spv";
    auto debugLineFragment = "Shaders/Compiled/DebugLines.frag.slang.spv";
    m_debugLinesShader = std::make_unique<VShader>(m_device,debugLineVertex,
                                             debugLineFragment);

    pipeline = std::make_unique<VGraphicsPipeline>(m_device, m_swapChain, *m_debugLinesShader, m_renderPass, m_pushDescriptorSetManager.GetLayout());
    pipeline->Init();
    pipeline->SetPipelineType(PIPELINE_TYPE_DEBUG_LINES);
    pipeline->SetPrimitiveTopology(vk::PrimitiveTopology::eLineStrip);
    m_pipelines[PIPELINE_TYPE_DEBUG_LINES] = std::move(pipeline);
}


