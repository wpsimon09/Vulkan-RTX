//
// Created by wpsimon09 on 31/10/24.
//

#include "VUniformBufferManager.hpp"

#include "Application/Client.hpp"
#include "Application/AssetsManger/Utils/VTextureAsset.hpp"
#include "Application/Rendering/Camera/Camera.hpp"
#include "Application/Rendering/Material/PBRMaterial.hpp"
#include "Application/Rendering/Mesh/StaticMesh.hpp"
#include "Application/Rendering/Transformations/Transformations.hpp"
#include "Application/Lightning/LightStructs.hpp"
#include "Application/Rendering/Scene/Scene.hpp"
#include "fastgltf/types.hpp"
#include "Vulkan/Global/GlobalState.hpp"
#include "Vulkan/Global/GlobalVariables.hpp"
#include "Vulkan/Global/VulkanStructs.hpp"
#include "Vulkan/VulkanCore/Buffer/VBuffer.hpp"
#include "Vulkan/VulkanCore/Device/VDevice.hpp"
#include "Vulkan/Utils/VUniformBufferManager/UnifromsRegistry.hpp"
#include "Vulkan/VulkanCore/Buffer/VShaderStorageBuffer.hpp"
#include "Vulkan/VulkanCore/Samplers/VSamplers.hpp"


#define MATERIAL_BUFFER_SIZE 5'242'880
#define PER_OBJECT_BUFFER_SIZE 5'242'880

VulkanUtils::VUniformBufferManager::VUniformBufferManager(const VulkanCore::VDevice& device)
    : m_device(device)
{
    Utils::Logger::LogInfoVerboseOnly("Creating uniform buffer manager...");
    CreateUniforms();
    Utils::Logger::LogSuccess("Uniform buffer manager created successfully");
}

const std::vector<vk::DescriptorBufferInfo>& VulkanUtils::VUniformBufferManager::GetGlobalBufferDescriptorInfo() const
{
    return m_perFrameUniform->GetDescriptorBufferInfos();
}

const std::vector<vk::DescriptorBufferInfo>& VulkanUtils::VUniformBufferManager::GetLightBufferDescriptorInfo() const
{
    return m_lightUniform->GetDescriptorBufferInfos();
}

vk::DescriptorBufferInfo VulkanUtils::VUniformBufferManager::GetPerObjectBuffer(int currentFrame) {
    vk::DescriptorBufferInfo bufferInfo;
    bufferInfo.buffer = m_perObjectData[currentFrame]->GetBuffer();
    bufferInfo.offset = 0;
    bufferInfo.range = m_perObjectData[currentFrame]->GetAllocatedSize();

    return bufferInfo;
}

std::vector<vk::DescriptorImageInfo> VulkanUtils::VUniformBufferManager::GetAll2DTextureDescriptorImageInfo() const
{
    std::vector<vk::DescriptorImageInfo> result;
    result.reserve(m_sceneTextures.size());
    for(auto& texture : m_sceneTextures)
    {
        assert(texture->GetHandle()->GetImageInfo().isStorage == false && "Image can not be storage buffer" );
        result.emplace_back(texture->GetHandle()->GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler2D));
    }
    return result;
}
vk::DescriptorBufferInfo VulkanUtils::VUniformBufferManager::GetMaterialDescriptionBuffer(int frameIndex) const
{
    vk::DescriptorBufferInfo descriptorBuffer;
    descriptorBuffer.buffer = m_sceneMaterials[frameIndex]->GetBuffer();
    descriptorBuffer.offset = 0;
    descriptorBuffer.range = m_sceneMaterials[frameIndex]->GetAllocatedSize();

    return  descriptorBuffer;
}


void VulkanUtils::VUniformBufferManager::UpdatePerFrameUniformData(int frameIndex, GlobalUniform& perFrameData) const
{
    m_perFrameUniform->GetUBOStruct() = perFrameData;
    auto view                         = perFrameData.view;
    auto projection                   = perFrameData.proj;
    m_perFrameUniform->GetUBOStruct().inverseView = glm::inverse(view);
    m_perFrameUniform->GetUBOStruct().inverseProj = glm::inverse(projection);

    m_perFrameUniform->UpdateGPUBuffer(frameIndex);
}

void VulkanUtils::VUniformBufferManager::UpdatePerObjectUniformData(int frameIndex,
                                                                    std::vector<std::pair<unsigned long, VulkanStructs::VDrawCallData>>& drawCalls) const
{
    int  i         = 0;

    if (drawCalls.empty()) return;

    //TODO: do not allocate new vector every time here instead allocate one that can fit at least 50% of the required buffer objects
    std::vector<PerObjectData> perObjectData (drawCalls.size());

    for(auto& drawCall : drawCalls)
    {
        drawCall.second.drawCallID = i;

        perObjectData[i].model = drawCall.second.modelMatrix;
        perObjectData[i].normalMatrix = glm::transpose(glm::inverse(drawCall.second.modelMatrix));
        perObjectData[i].position = glm::vec4(drawCall.second.position,1.0);
        perObjectData[i].indexes.x =  drawCall.second.materialIndex;

        i++;
    }

    m_perObjectData[frameIndex]->Update(perObjectData);

    m_currentDrawCalls = drawCalls.size();
}

void VulkanUtils::VUniformBufferManager::UpdateLightUniformData(int frameIndex, LightStructs::SceneLightInfo& sceneLightInfo) const
{

    if(sceneLightInfo.DirectionalLightInfo)
    {
        m_lightUniform->GetUBOStruct().directionalLight.colour = sceneLightInfo.DirectionalLightInfo->colour;
        m_lightUniform->GetUBOStruct().directionalLight.direction =
            glm::vec4(sceneLightInfo.DirectionalLightInfo->direction, 1.0f);
    }


    int numIterations = sceneLightInfo.PointLightInfos.size() <= 20 ? sceneLightInfo.PointLightInfos.size() : 20;
    for(int i = 0; i < numIterations; i++)
    {
        if(sceneLightInfo.PointLightInfos[i] != nullptr)
        {
            auto& pointLight                                      = sceneLightInfo.PointLightInfos[i];
            m_lightUniform->GetUBOStruct().pointLight[i].colour   = pointLight->colour;
            m_lightUniform->GetUBOStruct().pointLight[i].position = glm::vec4(pointLight->position, 1.F);
            m_lightUniform->GetUBOStruct().pointLight[i].CLQU_Parameters =
                glm::vec4(1.0f, pointLight->linearFactor, pointLight->quadraticFactor, pointLight->useAdvancedAttentuation);
        }
    }

    numIterations = sceneLightInfo.AreaLightInfos.size() <= 20 ? sceneLightInfo.AreaLightInfos.size() : 20;
    for(int i = 0; i < numIterations; i++)
    {
        if(sceneLightInfo.AreaLightInfos[i] != nullptr)
        {
            auto& areaLight                                        = sceneLightInfo.AreaLightInfos[i];
            m_lightUniform->GetUBOStruct().areaLights[i].edges     = areaLight->edges;
            m_lightUniform->GetUBOStruct().areaLights[i].intensity = glm::vec4(areaLight->colour, areaLight->intensity);
            m_lightUniform->GetUBOStruct().areaLights[i].twoSided  = areaLight->twoSided;
            m_lightUniform->GetUBOStruct().areaLights[i].isInUse   = areaLight->isAreaLightInUse;
        }
    }
    if(sceneLightInfo.environmentLight != nullptr)
    {
        m_lightUniform->GetUBOStruct().info.x =
            (sceneLightInfo.environmentLight->inUse && (sceneLightInfo.environmentLight->hdrImage != nullptr));
        m_lightUniform->GetUBOStruct().info.y = sceneLightInfo.environmentLight->ambientIntensity;
    }
    else
    {
        m_lightUniform->GetUBOStruct().info.y = sceneLightInfo.ambientStrenght;
        m_lightUniform->GetUBOStruct().info.x = static_cast<int>(false);
    }
    m_lightUniform->UpdateGPUBuffer(frameIndex);
}

void VulkanUtils::VUniformBufferManager::UpdateSceneDataInfo(int frameIndex, const ApplicationCore::SceneData& sceneData)
{
    std::vector<PBRMaterialDescription> materials(sceneData.pbrMaterials.size());
    for(int i = 0; i < sceneData.pbrMaterials.size(); i++) {
        materials[i] = *sceneData.pbrMaterials[i];
    }
    m_sceneMaterials[frameIndex]->Update(materials);
    m_sceneTextures = sceneData.textures;
}


void VulkanUtils::VUniformBufferManager::Destroy() const
{
    Utils::Logger::LogInfoVerboseOnly("Destroying uniform buffer manager and all its data...");
    m_perFrameUniform->Destory();
    m_lightUniform->Destory();


    for (auto& ssbo : m_sceneMaterials) {
        ssbo->Destroy();
    }

    for (auto& perObjectUbo : m_perObjectData) {
        perObjectUbo->Destroy();
    }

    Utils::Logger::LogInfoVerboseOnly("Uniform buffer manager destroyed");
}

void VulkanUtils::VUniformBufferManager::CreateUniforms()
{
    // allocate per model Uniform buffers
    Utils::Logger::LogInfoVerboseOnly("Allocating 100 uniform buffers before hand");
    GlobalState::DisableLogging();
    Utils::Logger::LogSuccess("Allocated 100 uniform buffers for per object data");

    m_sceneMaterials.resize(GlobalVariables::MAX_FRAMES_IN_FLIGHT);
    m_perObjectData.resize(GlobalVariables::MAX_FRAMES_IN_FLIGHT);

    for (int i = 0; i < GlobalVariables::MAX_FRAMES_IN_FLIGHT; i++) {
        m_sceneMaterials[i] = std::make_unique<VulkanCore::VShaderStorageBuffer>(m_device, MATERIAL_BUFFER_SIZE);
        m_sceneMaterials[i]->Allocate();

        m_perObjectData[i] = std::make_unique<VulkanCore::VShaderStorageBuffer>(m_device, PER_OBJECT_BUFFER_SIZE);
        m_perObjectData[i]->Allocate();
    }

    //assert(m_objectDataUniforms.size() == MAX_UBO_COUNT && "Failed to allocate 20 buffers");
    GlobalState::EnableLogging();
    Utils::Logger::LogSuccess("Allocated uniform and storage buffers");

    m_perFrameUniform = std::make_unique<VUniform<GlobalUniform>>(m_device);

    m_lightUniform = std::make_unique<VUniform<LightUniforms>>(m_device);
}
