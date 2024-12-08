//
// Created by wpsimon09 on 31/10/24.
//

#include "VUniformBufferManager.hpp"

#include "Application/Client.hpp"
#include "Application/Rendering/Camera/Camera.hpp"
#include "Application/Rendering/Mesh/Mesh.hpp"
#include "Application/Rendering/Transformations/Transformations.hpp"
#include "Vulkan/Global/GlobalState.hpp"
#include "Vulkan/Global/GlobalVariables.hpp"
#include "Vulkan/Global/VulkanStructs.hpp"
#include "Vulkan/VulkanCore/Buffer/VBuffer.hpp"
#include "Vulkan/VulkanCore/Device/VDevice.hpp"

#define MAX_UBO_COUNT 10

VulkanUtils::VUniformBufferManager::VUniformBufferManager(const VulkanCore::VDevice &device, const Client &client):m_device(device),m_client(client) {
    Utils::Logger::LogInfoVerboseOnly("Creating uniform buffer manager...");
    CreateUniforms();
    Utils::Logger::LogSuccess("Uniform buffer manager created successfully");
}

const std::vector<vk::DescriptorBufferInfo> &VulkanUtils::VUniformBufferManager::GetGlobalBufferDescriptorInfo() const {
    return m_cameraUniform->GetDescriptorBufferInfos();
}

const std::vector<vk::DescriptorBufferInfo>& VulkanUtils::VUniformBufferManager::GetMaterialFeaturesDescriptorBufferInfo(
    int meshIndex) const
{
    // returns 2 buffer descriptor info for each frame in flight
    return m_materialFeaturesUniform[meshIndex]->GetDescriptorBufferInfos();
}

const std::vector<vk::DescriptorBufferInfo>& VulkanUtils::VUniformBufferManager::
GetPerMaterialNoMaterialDescrptorBufferInfo(int meshIndex) const
{
    return m_materialNoTextureUniform[meshIndex]->GetDescriptorBufferInfos();
}

const std::vector<vk::DescriptorBufferInfo> & VulkanUtils::VUniformBufferManager::GetPerObjectDescriptorBufferInfo(
    int meshIndex) const {
    // returns 2 buffer descriptor info for each frame in flight
    return m_objectDataUniforms[meshIndex]->GetDescriptorBufferInfos();
}

void VulkanUtils::VUniformBufferManager::UpdateAllUniformBuffers(int frameIndex,
    std::vector<VulkanStructs::DrawCallData>& drawCalls) const
{
    m_cameraUniform->GetUBOStruct().proj = m_client.GetCamera().GetProjectionMatrix();
    m_cameraUniform->GetUBOStruct().view = m_client.GetCamera().GetViewMatrix();
    m_cameraUniform->GetUBOStruct().inverseView = m_client.GetCamera().GetInverseViewMatrix();

    m_cameraUniform->GetUBOStruct().screenSize = m_client.GetCamera().GetScreenSize();

    m_cameraUniform->GetUBOStruct().playerPosition = glm::vec4(m_client.GetCamera().GetPosition(),1.0f);
    if (m_client.GetIsRTXOn())
    {
        m_cameraUniform->GetUBOStruct().lightPosition = m_client.GetLightPosition();
        m_cameraUniform->GetUBOStruct().lightPosition.y *= -1.0f;
    }else
    {
        m_cameraUniform->GetUBOStruct().lightPosition = m_client.GetLightPosition();

    }
    m_cameraUniform->GetUBOStruct().viewParams = glm::vec4(m_client.GetCamera().GetCameraPlaneWidthAndHeight(), m_client.GetCamera().GetNearPlane(),1.0f);
    m_cameraUniform->UpdateGPUBuffer(frameIndex);

    for (int i = 0; i< drawCalls.size(); i++) {
        m_materialFeaturesUniform[i]->GetUBOStruct() = drawCalls[i].material->GetMaterialDescription().features;
        m_materialFeaturesUniform[i]->UpdateGPUBuffer(frameIndex);

        m_materialNoTextureUniform[i]->GetUBOStruct() = drawCalls[i].material->GetMaterialDescription().values;
        m_materialNoTextureUniform[i]->UpdateGPUBuffer(frameIndex);

        m_objectDataUniforms[i]->GetUBOStruct().model = drawCalls[i].modelMatrix;
        m_objectDataUniforms[i]->GetUBOStruct().normalMatrix = glm::transpose(glm::inverse( drawCalls[i].modelMatrix));
        m_objectDataUniforms[i]->UpdateGPUBuffer(frameIndex);
    }
}

void VulkanUtils::VUniformBufferManager::Destroy() const {
    Utils::Logger::LogInfoVerboseOnly("Destroying uniform buffer manager and all its data...");
    m_cameraUniform->Destory();
    for(auto &ubo: m_objectDataUniforms) {
        ubo->Destory();
    }
    for (auto& mat: m_materialFeaturesUniform)
    {
        mat->Destory();
    }
    for (auto& mat: m_materialNoTextureUniform)
    {
        mat->Destory();
    }
    Utils::Logger::LogInfoVerboseOnly("Uniform buffer manager destroyed");
}

void VulkanUtils::VUniformBufferManager::CreateUniforms() {

    // allocate per model Uniform buffers
    Utils::Logger::LogInfoVerboseOnly("Allocating 100 uniform buffers before hand");
    GlobalState::LoggingEnabled = false;
    Utils::Logger::LogSuccess("Allocated 100 uniform buffers for per object data");

    m_objectDataUniforms.resize(MAX_UBO_COUNT);
    m_materialFeaturesUniform.resize(MAX_UBO_COUNT);

    m_materialNoTextureUniform.resize(MAX_UBO_COUNT);

    for(int i = 0; i <MAX_UBO_COUNT; i++) {
        m_objectDataUniforms[i] = (std::make_unique<VUniform<PerObjectUBO::ObjectDataUniform>>(m_device));
    }

    for(int i = 0; i <MAX_UBO_COUNT; i++) {
        m_materialFeaturesUniform[i] = (std::make_unique<VUniform<PBRMaterialFeaturees>>(m_device));
    }

    for(int i = 0; i <MAX_UBO_COUNT; i++) {
        m_materialNoTextureUniform[i] = (std::make_unique<VUniform<PBRMaterialNoTexture>>(m_device));
    }

    //assert(m_objectDataUniforms.size() == MAX_UBO_COUNT && "Failed to allocate 20 buffers");
    GlobalState::LoggingEnabled = true;
    Utils::Logger::LogSuccess("Allocated 100 uniform buffers for each of the mesh");


    // allocate per Frame uniform buffers
    m_cameraUniform = std::make_unique<VUniform<PerFrameUBO::GlobalUniform>>(m_device);
}