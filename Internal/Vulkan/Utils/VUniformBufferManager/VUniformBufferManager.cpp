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


#define MAX_UBO_COUNT 1000
#define MATERIAL_BUFFER_SIZE 16'777'216

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


const std::vector<vk::DescriptorBufferInfo>& VulkanUtils::VUniformBufferManager::GetPerObjectDescriptorBufferInfo(int meshIndex) const
{
    // returns 2 buffer descriptor info for each frame in flight
    return m_perObjectUniform[meshIndex]->GetDescriptorBufferInfos();
}
std::vector<vk::DescriptorImageInfo> VulkanUtils::VUniformBufferManager::GetAll2DTextureDescriptorImageInfo(
    const ApplicationCore::SceneData& sceneData) const
{
    std::vector<vk::DescriptorImageInfo> result;
    result.reserve(sceneData.textures.size());
    for(auto& texture : sceneData.textures)
    {
        result.emplace_back(texture->GetHandle()->GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler2D));
    }
    return result;
}
vk::DescriptorBufferInfo VulkanUtils::VUniformBufferManager::GetSceneBufferDescriptorInfo(int frameIndex) const
{
    vk::DescriptorBufferInfo descriptorBuffer;
    descriptorBuffer.buffer = m_rtxMaterialDescriptions[frameIndex]->GetBuffer();
    descriptorBuffer.offset = 0;
    descriptorBuffer.range = vk::WholeSize;

    return  descriptorBuffer;
}


void VulkanUtils::VUniformBufferManager::UpdatePerFrameUniformData(int frameIndex, GlobalUniform& perFrameData) const
{
    m_perFrameUniform->GetUBOStruct() = perFrameData;
    auto view                         = perFrameData.view;
    auto projection                         = perFrameData.proj;
    m_perFrameUniform->GetUBOStruct().inverseView = glm::inverse(view);
    m_perFrameUniform->GetUBOStruct().inverseProj = glm::inverse(projection);

    m_perFrameUniform->UpdateGPUBuffer(frameIndex);
}

void VulkanUtils::VUniformBufferManager::UpdatePerObjectUniformData(int frameIndex,
                                                                    std::vector<std::pair<unsigned long, VulkanStructs::VDrawCallData>>& drawCalls) const
{
    assert(drawCalls.size() < MAX_UBO_COUNT && "Draw calls are bigger than allocated uniform buffers on GPU");
    int  i         = 0;
    bool reloadAll = m_currentDrawCalls != drawCalls.size();

    for(auto& drawCall : drawCalls)
    {
        drawCall.second.drawCallID = i;

        //if (drawCall.second != m_perObjectUniform[i]->GetUBOStruct() || reloadAll)
        {
            m_perObjectUniform[i]->GetUBOStruct().model    = drawCall.second.modelMatrix;
            m_perObjectUniform[i]->GetUBOStruct().position = drawCall.second.position;

            if(auto mat = dynamic_cast<ApplicationCore::PBRMaterial*>(drawCall.second.material))
            {
                m_perObjectUniform[i]->GetUBOStruct().material.features = mat->GetMaterialDescription().features;
                m_perObjectUniform[i]->GetUBOStruct().material.values   = mat->GetMaterialDescription().values;
            }

            m_perObjectUniform[i]->UpdateGPUBuffer(frameIndex);
        }
        i++;
    }

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

void VulkanUtils::VUniformBufferManager::UpdateSceneDataInfo(int frameIndex, const ApplicationCore::SceneData& sceneData) const
{
    std::vector<PBRMaterialDescription> materials(sceneData.pbrMaterials.size());
    for(int i = 0; i < sceneData.pbrMaterials.size(); i++) {
        materials[i] = *sceneData.pbrMaterials[i];
    }
    m_rtxMaterialDescriptions[frameIndex]->Update(materials);
}


void VulkanUtils::VUniformBufferManager::Destroy() const
{
    Utils::Logger::LogInfoVerboseOnly("Destroying uniform buffer manager and all its data...");
    m_perFrameUniform->Destory();
    m_lightUniform->Destory();
    for(auto& ubo : m_perObjectUniform)
    {
        ubo->Destory();
    }

    for (auto& ssbo : m_rtxMaterialDescriptions) {
        ssbo->Destroy();
    }

    Utils::Logger::LogInfoVerboseOnly("Uniform buffer manager destroyed");
}

void VulkanUtils::VUniformBufferManager::CreateUniforms()
{
    // allocate per model Uniform buffers
    Utils::Logger::LogInfoVerboseOnly("Allocating 100 uniform buffers before hand");
    GlobalState::DisableLogging();
    Utils::Logger::LogSuccess("Allocated 100 uniform buffers for per object data");

    m_perObjectUniform.resize(MAX_UBO_COUNT);
    m_rtxMaterialDescriptions.resize(GlobalVariables::MAX_FRAMES_IN_FLIGHT);

    for(int i = 0; i < MAX_UBO_COUNT; i++)
    {
        m_perObjectUniform[i] = (std::make_unique<VUniform<ObjectDataUniform>>(m_device));

        //m_rayTracingMaterials[i] = (std::make_unique<VUniform<PBRMaterialDescription>>(m_device));
    }

    m_rtxMaterialDescriptions.resize(GlobalVariables::MAX_FRAMES_IN_FLIGHT);
    for (int i = 0; i < GlobalVariables::MAX_FRAMES_IN_FLIGHT; i++) {
        m_rtxMaterialDescriptions[i] = std::make_unique<VulkanCore::VShaderStorageBuffer>(m_device, MATERIAL_BUFFER_SIZE);
        m_rtxMaterialDescriptions[i]->Allocate();
    }

    //assert(m_objectDataUniforms.size() == MAX_UBO_COUNT && "Failed to allocate 20 buffers");
    GlobalState::EnableLogging();
    Utils::Logger::LogSuccess("Allocated 100 uniform buffers for each of the mesh");


    // allocate per Frame uniform buffers
    m_perFrameUniform = std::make_unique<VUniform<GlobalUniform>>(m_device);

    m_lightUniform = std::make_unique<VUniform<LightUniforms>>(m_device);
}
