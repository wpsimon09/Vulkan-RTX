//
// Created by wpsimon09 on 31/10/24.
//

#include "VUniformBufferManager.hpp"

#include "Application/Client.hpp"
#include "Application/Rendering/Camera/Camera.hpp"
#include "Application/Rendering/Mesh/StaticMesh.hpp"
#include "Application/Rendering/Transformations/Transformations.hpp"
#include "fastgltf/types.hpp"
#include "Vulkan/Global/GlobalState.hpp"
#include "Vulkan/Global/GlobalVariables.hpp"
#include "Vulkan/Global/VulkanStructs.hpp"
#include "Vulkan/VulkanCore/Buffer/VBuffer.hpp"
#include "Vulkan/VulkanCore/Device/VDevice.hpp"

#define MAX_UBO_COUNT 1000

VulkanUtils::VUniformBufferManager::VUniformBufferManager(const VulkanCore::VDevice &device):m_device(device) {
    Utils::Logger::LogInfoVerboseOnly("Creating uniform buffer manager...");
    CreateUniforms();
    Utils::Logger::LogSuccess("Uniform buffer manager created successfully");
}

const std::vector<vk::DescriptorBufferInfo> &VulkanUtils::VUniformBufferManager::GetGlobalBufferDescriptorInfo() const {
    return m_perFrameUniform->GetDescriptorBufferInfos();
}

const std::vector<vk::DescriptorBufferInfo>& VulkanUtils::VUniformBufferManager::GetLightBufferDescriptorInfo() const
{
    return m_lightUniform->GetDescriptorBufferInfos();
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
    return m_perObjectUniform[meshIndex]->GetDescriptorBufferInfos();
}

void VulkanUtils::VUniformBufferManager::UpdatePerFrameUniformData(int frameIndex, GlobalUniform& perFrameData) const
{

    m_perFrameUniform->GetUBOStruct() = perFrameData;
    m_perFrameUniform->UpdateGPUBuffer(frameIndex);
}

void VulkanUtils::VUniformBufferManager::UpdatePerObjectUniformData(int frameIndex,
    std::vector<VulkanStructs::DrawCallData*>& drawCalls) const
{
    assert(drawCalls.size() < MAX_UBO_COUNT && "Draw calls are bigger than allocated uniform buffers on GPU");
    for (int i = 0; i < drawCalls.size(); i++)
    {
        drawCalls[i]->drawCallID = i;

        m_perObjectUniform[i]->GetUBOStruct().model = drawCalls[i]->modelMatrix;
        m_perObjectUniform[i]->GetUBOStruct().normalMatrix = glm::transpose(glm::inverse( drawCalls[i]->modelMatrix));
        m_perObjectUniform[i]->GetUBOStruct().position = drawCalls[i]->position;
        m_perObjectUniform[i]->UpdateGPUBuffer(frameIndex);

        m_materialFeaturesUniform[i]->GetUBOStruct() = drawCalls[i]->material->GetMaterialDescription().features;
        m_materialFeaturesUniform[i]->UpdateGPUBuffer(frameIndex);

        m_materialNoTextureUniform[i]->GetUBOStruct() = drawCalls[i]->material->GetMaterialDescription().values;
        m_materialNoTextureUniform[i]->UpdateGPUBuffer(frameIndex);
    }

}

void VulkanUtils::VUniformBufferManager::UpdateLightUniformData(int frameIndex,
    LightStructs::SceneLightInfo& sceneLightInfo) const
{
    if (sceneLightInfo.DirectionalLightInfo)
    {
        m_lightUniform->GetUBOStruct().directionalLight.colour = sceneLightInfo.DirectionalLightInfo->colour;
        m_lightUniform->GetUBOStruct().directionalLight.direction = glm::vec4(sceneLightInfo.DirectionalLightInfo->direction,1.0f);
    }


    int numIterations =  sceneLightInfo.PointLightInfos.size() <= 20 ?  sceneLightInfo.PointLightInfos.size() : 20;
    for (int i =0; i<numIterations ; i++)
    {LTC_ImageData
        if (sceneLightInfo.PointLightInfos[i] != nullptr)
        {
            auto& pointLight = sceneLightInfo.PointLightInfos[i];
            m_lightUniform->GetUBOStruct().pointLight[i].colour = pointLight->colour;
            m_lightUniform->GetUBOStruct().pointLight[i].position = glm::vec4(pointLight->position,1.F);
            m_lightUniform->GetUBOStruct().pointLight[i].CLQU_Parameters = glm::vec4(
                1.0f, pointLight->linearFactor, pointLight->quadraticFactor, pointLight->useAdvancedAttentuation
            );
        }
    }

    m_lightUniform->UpdateGPUBuffer(frameIndex);
}


void VulkanUtils::VUniformBufferManager::UpdatePerMaterialUniformData(int frameIndex,
                                                                      const std::shared_ptr<ApplicationCore::Material>& material) const
{

}


void VulkanUtils::VUniformBufferManager::Destroy() const {
    Utils::Logger::LogInfoVerboseOnly("Destroying uniform buffer manager and all its data...");
    m_perFrameUniform->Destory();
    m_lightUniform->Destory();
    for(auto &ubo: m_perObjectUniform) {
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
    GlobalState::DisableLogging();
    Utils::Logger::LogSuccess("Allocated 100 uniform buffers for per object data");

    m_perObjectUniform.resize(MAX_UBO_COUNT);
    m_materialFeaturesUniform.resize(MAX_UBO_COUNT);

    m_materialNoTextureUniform.resize(MAX_UBO_COUNT);

    for(int i = 0; i <MAX_UBO_COUNT; i++) {
        m_perObjectUniform[i] = (std::make_unique<VUniform<ObjectDataUniform>>(m_device));
    }

    for(int i = 0; i <MAX_UBO_COUNT; i++) {
        m_materialFeaturesUniform[i] = (std::make_unique<VUniform<PBRMaterialFeaturees>>(m_device));
    }

    for(int i = 0; i <MAX_UBO_COUNT; i++) {
        m_materialNoTextureUniform[i] = (std::make_unique<VUniform<PBRMaterialNoTexture>>(m_device));
    }



    //assert(m_objectDataUniforms.size() == MAX_UBO_COUNT && "Failed to allocate 20 buffers");
    GlobalState::EnableLogging();
    Utils::Logger::LogSuccess("Allocated 100 uniform buffers for each of the mesh");


    // allocate per Frame uniform buffers
    m_perFrameUniform = std::make_unique<VUniform<GlobalUniform>>(m_device);

    m_lightUniform = std::make_unique<VUniform<LightUniforms>>(m_device);
}
