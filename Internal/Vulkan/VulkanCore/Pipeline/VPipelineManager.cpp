//
// Created by wpsimon09 on 06/10/24.
//

#include "VPipelineManager.hpp"


VulkanCore::VPipelineManager::VPipelineManager(const VulkanCore::VDevice &device,
    const VulkanCore::VSwapChain &swapChain):m_device(device), m_swapChain(swapChain) {

    ConfigurePipelines();
    CreatePipelines();
}

void VulkanCore::VPipelineManager::DestoryPipelines() {
}


void VulkanCore::VPipelineManager::CreatePipelines() {
}

void VulkanCore::VPipelineManager::ConfigurePipelines() {

}
