//
// Created by wpsimon09 on 31/10/24.
//

#include "VUniformBufferManager.hpp"

#include "Application/Client.hpp"
#include "Vulkan/Global/GlobalVariables.hpp"
#include "Vulkan/VulkanCore/Buffer/VBuffer.hpp"
#include "Vulkan/VulkanCore/Device/VDevice.hpp"


VulkanUtils::VUniformBufferManager::VUniformBufferManager(const VulkanCore::VDevice &device, const Client &client):m_device(device),m_client(client) {
    Utils::Logger::LogInfoVerboseOnly("Creating uniform buffer manager...");
    CreateUniforms();
    Utils::Logger::LogSuccess("Uniform buffer manager created successfully");
}

const std::vector<vk::DescriptorBufferInfo> &VulkanUtils::VUniformBufferManager::GetGlobalBufferDescriptorInfo() const {
    return m_cameraUniform->GetDescriptorBufferInfos();
}

void VulkanUtils::VUniformBufferManager::UpdateAllUniformBuffers(int frameIndex) {

}

void VulkanUtils::VUniformBufferManager::Destroy() const {
    Utils::Logger::LogInfoVerboseOnly("Destroying uniform buffer manager...");

    Utils::Logger::LogInfoVerboseOnly("Uniform buffer manager destroyed");
}

void VulkanUtils::VUniformBufferManager::CreateUniforms() {
    m_cameraUniform = std::make_unique<VUniform<PerFrameUBO::CameraUniform>>(m_device);
}
