//
// Created by wpsimon09 on 06/10/24.
//

#include "VPipelineManager.hpp"

#include "VGraphicsPipeline.hpp"
#include "Application/Logger/Logger.hpp"
#include "Vulkan/VulkanCore/Shader/VShader.hpp"
#include "Vulkan/VulkanCore/SwapChain/VSwapChain.hpp"


VulkanCore::VPipelineManager::VPipelineManager(const VulkanCore::VDevice &device,
                                               const VulkanCore::VSwapChain &swapChain,
                                               const VulkanCore::VRenderPass &renderPass) :
    m_device(device), m_swapChain(swapChain), m_renderPass(renderPass) {
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

void VulkanCore::VPipelineManager::GeneratePipelines() {

    auto basicPipelineShaderVertexSource = "Shaders/Compiled/BaseTriangleVertex.spv";
    auto basicPipelineFragmentShaderSource = "Shaders/Compiled/BaseTriangleFragment.spv";
    m_baseShader = std::make_unique<VShader>(m_device, basicPipelineShaderVertexSource,
                                             basicPipelineFragmentShaderSource);
    auto basicPipeline = std::make_unique<VGraphicsPipeline>(m_device, m_swapChain, *m_baseShader, m_renderPass);
    basicPipeline->Init();
    basicPipeline->SetPipelineType(PIPELINE_TYPE_RASTER_BASIC);
    SpecifyPipelineCommands(*basicPipeline);
    m_pipelines.emplace(std::make_pair(PIPELINE_TYPE_RASTER_BASIC, std::move(basicPipeline)));
}

void VulkanCore::VPipelineManager::SpecifyPipelineCommands(VGraphicsPipeline &pipeline) {


    switch (pipeline.GetPipelineType()) {
    case PIPELINE_TYPE_RASTER_BASIC: {
        pipeline.AddCommand([&](vk::CommandBuffer cmd) {
            cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline.GetPipelineInstance());
            vk::Viewport viewport = {};
            viewport.x = 0.0f;
            viewport.y = 0.0f;
            viewport.width = static_cast<float>(m_swapChain.GetExtent().width);
            viewport.height = static_cast<float>(m_swapChain.GetExtent().height);
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;
            cmd.setViewport(0, 1, &viewport);

            vk::Rect2D scissors;
            scissors.offset.x = 0;
            scissors.offset.y = 0;
            scissors.extent = m_swapChain.GetExtent();
            cmd.setScissor(0, 1, &scissors);

            cmd.draw(3, 1,1, 0);
            Utils::Logger::LogInfoVerboseOnly("Specified commands for the BasicRasterPipeline");

        });

        break;
    }
    }
}
