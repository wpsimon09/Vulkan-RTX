//
// Created by wpsimon09 on 06/10/24.
//

#include "VPipelineManager.hpp"




VulkanCore::VPipelineManager::VPipelineManager(const VulkanCore::VDevice &device,
    const VulkanCore::VSwapChain &swapChain, const VulkanCore::VRenderPass &renderPass)
        :m_device(device), m_swapChain(swapChain), m_renderPass(m_renderPass){
}

void VulkanCore::VPipelineManager::DestoryPipelines() {
}


void VulkanCore::VPipelineManager::CreatePipelines() {
}

std::vector<vk::GraphicsPipelineCreateInfo> VulkanCore::VPipelineManager::ConfigurePipelines() {

}
