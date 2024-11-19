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

    auto basicPipelineShaderVertexSource = "Shaders/Compiled/BaseTriangleVertex.spv";
    auto basicPipelineFragmentShaderSource = "Shaders/Compiled/BaseTriangleFragment.spv";
    m_baseShader = std::make_unique<VShader>(m_device, basicPipelineShaderVertexSource,
                                             basicPipelineFragmentShaderSource);
    auto basicPipeline = std::make_unique<VGraphicsPipeline>(m_device, m_swapChain, *m_baseShader, m_renderPass, m_pushDescriptorSetManager.GetLayout());
    basicPipeline->Init();
    basicPipeline->SetPipelineType(PIPELINE_TYPE_RASTER_PBR_TEXTURED);
    m_pipelines.insert(std::make_pair(PIPELINE_TYPE_RASTER_PBR_TEXTURED, std::move(basicPipeline)));

    basicPipelineShaderVertexSource = "Shaders/Compiled/BaseTriangleVertex.spv";
    basicPipelineFragmentShaderSource = "Shaders/Compiled/GGXColorFragment.spv";
    m_baseShader = std::make_unique<VShader>(m_device, basicPipelineShaderVertexSource,
                                             basicPipelineFragmentShaderSource);
    basicPipeline = std::make_unique<VGraphicsPipeline>(m_device, m_swapChain, *m_baseShader, m_renderPass, m_pushDescriptorSetManager.GetLayout());
    basicPipeline->Init();
    basicPipeline->SetPipelineType(PIPELINE_TYPE_RASTER_PBR_COLOURED);
    m_pipelines.insert(std::make_pair(PIPELINE_TYPE_RASTER_PBR_COLOURED, std::move(basicPipeline)));

}


