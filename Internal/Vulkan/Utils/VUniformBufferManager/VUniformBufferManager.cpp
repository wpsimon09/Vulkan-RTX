//
// Created by wpsimon09 on 31/10/24.
//

#include "VUniformBufferManager.hpp"

#include "Application/Client.hpp"
#include "Application/Rendering/Camera/Camera.hpp"
#include "Application/Rendering/Mesh/Mesh.hpp"
#include "Application/Rendering/Transformations/Transformations.hpp"
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


void VulkanUtils::VUniformBufferManager::UpdateAllUniformBuffers(int frameIndex) const {
    m_cameraUniform->GetUBOStruct().proj = m_client.GetCamera().GetProjectionMatrix();
    m_cameraUniform->GetUBOStruct().view = m_client.GetCamera().GetViewMatrix();
    m_cameraUniform->UpdateGPUBuffer(frameIndex);

    for (int i = 0; i< m_client.GetMeshes().size(); i++) {
        m_objectDataUnifrom[i]->GetUBOStruct().model = m_client.GetMeshes()[i].get().GetTransformations().GetModelMatrix();
    }



}

void VulkanUtils::VUniformBufferManager::Destroy() const {
    Utils::Logger::LogInfoVerboseOnly("Destroying uniform buffer manager...");
    m_cameraUniform->Destory();
    for(auto &ubo: m_objectDataUnifrom) {
        ubo->Destory();
    }
    Utils::Logger::LogInfoVerboseOnly("Uniform buffer manager destroyed");
}

void VulkanUtils::VUniformBufferManager::CreateUniforms() {

    // allocate per model Uniform buffers
    Utils::Logger::LogInfoVerboseOnly("Allocating 100 uniform buffers before hand");
    m_objectDataUnifrom.resize(100);
    assert(m_objectDataUnifrom.size() == 100 && "Failed to allocate 100 buffers");
    Utils::Logger::LogSuccess("Allocated 100 uniform buffers for per object data");
    std::generate_n(m_objectDataUnifrom.begin(), 100, [&]() {
        return std::make_unique<VUniform<PerObjectUBO::ObjectDataUniform>>(m_device);
    });

    // allocate per Frame uniform buffers
    m_cameraUniform = std::make_unique<VUniform<PerFrameUBO::CameraUniform>>(m_device);
}