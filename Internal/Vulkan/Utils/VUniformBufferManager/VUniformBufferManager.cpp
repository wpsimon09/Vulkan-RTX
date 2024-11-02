//
// Created by wpsimon09 on 31/10/24.
//

#include "VUniformBufferManager.hpp"

#include "Vulkan/Global/GlobalVariables.hpp"
#include "Vulkan/VulkanCore/Buffer/VBuffer.hpp"
#include "Vulkan/VulkanCore/Device/VDevice.hpp"


VulkanUtils::VUniformBufferManager::VUniformBufferManager(const VulkanCore::VDevice &device, const Client &client):m_device(device),m_client(client) {
    Utils::Logger::LogInfoVerboseOnly("Creating uniform buffer manager...");
    CreateUniforms();
    Utils::Logger::LogSuccess("Uniform buffer manager created successfully");
}

std::vector<vk::DescriptorBufferInfo *> &VulkanUtils::VUniformBufferManager::GetGlobalBufferDescriptorInfo() const {
    return m_cameraUniform->descriptorBufferInfo;
}

void VulkanUtils::VUniformBufferManager::Destroy() const {
    Utils::Logger::LogInfoVerboseOnly("Destroying uniform buffer manager...");
    for (auto &buffer : m_cameraUniform->buffer) {
        buffer->Destroy();
    }
    Utils::Logger::LogInfoVerboseOnly("Uniform buffer manager destroyed");
}

void VulkanUtils::VUniformBufferManager::CreateUniforms() {
    m_cameraUniform = std::make_unique<PerFrameUBO::CameraUniform>();
    m_cameraUniform->buffer.resize(GlobalVariables::MAX_FRAMES_IN_FLIGHT);
    m_cameraUniform->descriptorBufferInfo.resize(GlobalVariables::MAX_FRAMES_IN_FLIGHT);
    for (int i = 0; i < GlobalVariables::MAX_FRAMES_IN_FLIGHT; i++) {
        m_cameraUniform->buffer[i] = std::make_unique<VulkanCore::VBuffer>(m_device);
        m_cameraUniform->buffer[i]->MakeUniformBuffer(*m_cameraUniform);
        m_cameraUniform->descriptorBufferInfo[i] = m_cameraUniform->buffer[i]->GetBufferInfoForDescriptor();
    }
}
