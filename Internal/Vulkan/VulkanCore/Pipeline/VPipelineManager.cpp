//
// Created by wpsimon09 on 06/10/24.
//

#include "VPipelineManager.hpp"

#include "VGraphicsPipeline.hpp"
#include "Application/Logger/Logger.hpp"
#include "Vulkan/VulkanCore/Shader/VShader.hpp"


VulkanCore::VPipelineManager::VPipelineManager(const VulkanCore::VDevice &device,
                                               const VulkanCore::VSwapChain &swapChain, const VulkanCore::VRenderPass &renderPass)
        :m_device(device), m_swapChain(swapChain), m_renderPass(renderPass){
}

void VulkanCore::VPipelineManager::DestroyPipelines() {
    for (auto &pipeline :m_pipelines) {
        pipeline.second->Destroy();
    }
}


void VulkanCore::VPipelineManager::InstantiatePipelines() {

    GeneratePipelines();
    Utils::Logger::LogInfoVerboseOnly("Creating " + std::to_string(m_pipelines.size()) + " graphics pipelines..." );

    std::vector<vk::GraphicsPipelineCreateInfo> graphicsPipelineCreateInfos;
    for(auto &pipeline: m_pipelines) {
        graphicsPipelineCreateInfos.emplace_back(pipeline.second->GetGraphicsPipelineCreateInfoStruct());
    }


    auto createdVkPipelines = m_device.GetDevice().createGraphicsPipelines(nullptr, graphicsPipelineCreateInfos);

    assert(createdVkPipelines.value.size() == m_pipelines.size());
    Utils::Logger::LogSuccess("Successfully created " + std::to_string(m_pipelines.size()) + "graphics pipelines");

    Utils::Logger::LogInfoVerboseOnly("Binding pipelines...");
    int i = 0;
    for(auto &pipeline: m_pipelines) {

        assert(createdVkPipelines.result == vk::Result::eSuccess);
        if(createdVkPipelines.result == vk::Result::ePipelineCompileRequired) {
            throw("Pipeline at index %d needs to be compiled", std::to_string(i).c_str());
        }

        pipeline.second->SetCreatedPipeline(createdVkPipelines.value[i]);
        i++;
    }
    Utils::Logger::LogSuccess("Successfully created " + std::to_string(i) + "graphics pipelines");


}

void VulkanCore::VPipelineManager::GeneratePipelines() {

    auto basicPipelineShaderVertexSource = "Shaders/Compiled/TriangleVertex.spv";
    auto basicPipelineFragmentShaderSource = "Shaders/Compiled/TriangleFragment.spv";
    VulkanCore::VShader shader(m_device, basicPipelineShaderVertexSource, basicPipelineFragmentShaderSource);
    auto basicPipeline = std::make_unique<VGraphicsPipeline>(m_device, m_swapChain,shader, m_renderPass  );
    basicPipeline->Init();

    m_pipelines.emplace(std::make_pair(PIPELINE_TYPE_RASTER_BASIC, std::move(basicPipeline)));
}

