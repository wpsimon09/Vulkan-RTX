//
// Created by wpsimon09 on 26/11/24.
//

#include "PBRMaterial.hpp"
#include "MaterialStructs.hpp"


#include "Application/AssetsManger/AssetsManager.hpp"
#include "Application/AssetsManger/Utils/VTextureAsset.hpp"
#include "Application/Structs/ApplicationStructs.hpp"
#include "Application/Structs/ApplicationStructs.hpp"
#include "Application/Structs/ApplicationStructs.hpp"
#include "Application/Structs/ApplicationStructs.hpp"
#include "Application/Utils/LinearyTransformedCosinesValues.hpp"
#include "Vulkan/Utils/VEffect/VRasterEffect.hpp"
#include "Vulkan/Utils/VUniformBufferManager/VUniformBufferManager.hpp"
#include "Vulkan/VulkanCore/Samplers/VSamplers.hpp"
#include "Vulkan/VulkanCore/VImage/VImage2.hpp"

namespace ApplicationCore {
PBRMaterial::PBRMaterial(std::shared_ptr<VulkanUtils::VRasterEffect> materialEffect, MaterialPaths& materialPaths, AssetsManager& assets_manager)
    : m_materialPaths(materialPaths)
    , BaseMaterial(materialEffect)
{

    if(!materialPaths.DiffuseMapPath.empty())
    {
        assets_manager.GetTexture(m_textures[ETextureType::Diffues], m_materialPaths.DiffuseMapPath, m_materialPaths.saveToDisk);
        m_materialDescription.features.hasDiffuseTexture = true;
    }
    else
        assets_manager.GetDummyTexture(m_textures[ETextureType::Diffues]);

    if(!materialPaths.NormalMapPath.empty())
    {
        assets_manager.GetTexture(m_textures[ETextureType::normal], m_materialPaths.NormalMapPath, m_materialPaths.saveToDisk);
        m_materialDescription.features.hasNormalTexture = true;
    }
    else
        assets_manager.GetDummyTexture(m_textures[ETextureType::normal]);

    if(!materialPaths.ArmMapPath.empty())
    {
        assets_manager.GetTexture(m_textures[ETextureType::arm], m_materialPaths.ArmMapPath, m_materialPaths.saveToDisk);
        m_materialDescription.features.hasArmTexture = true;
    }
    else
        assets_manager.GetDummyTexture(m_textures[ETextureType::arm]);

    if(!materialPaths.EmmisivePath.empty())
    {
        assets_manager.GetTexture(m_textures[ETextureType::Emissive], m_materialPaths.EmmisivePath, m_materialPaths.saveToDisk);
        m_materialDescription.features.hasEmissiveTexture = true;
    }
    else
    {
        assets_manager.GetDummyTexture(m_textures[ETextureType::Emissive]);
        m_materialDescription.features.hasEmissiveTexture = false;
    }
}

void UpdateGPU(VulkanUtils::DescriptorSetTemplateVariant updateStruct) {}


void PBRMaterial::ResetEffect()
{
    m_materialEffect = m_initialEffect;
}

void PBRMaterial::UpdateGPUTextureData(VulkanUtils::DescriptorSetTemplateVariantRef updateStruct, int frame)
{
    std::visit(
        [this, frame](auto& descriptorTemplateStruct) {
            auto& effectDstStruct = descriptorTemplateStruct.get();
            using T               = std::decay_t<decltype(effectDstStruct)>;

            if constexpr(std::is_same_v<T, VulkanUtils::BasicDescriptorSet>)
            {
            }
            else if constexpr(std::is_same_v<T, VulkanUtils::Unlit>)
            {
                m_materialEffect->WriteImage(
                    frame, 0, 3, m_textures[Diffues]->GetHandleByRef().GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler2D));
            }
            else if constexpr(std::is_same_v<T, VulkanUtils::ForwardShadingDstSet>)
            {

                m_materialEffect->WriteImage(
                    frame, 0, 6, m_textures[Diffues]->GetHandleByRef().GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler2D));
                m_materialEffect->WriteImage(
                    frame, 0, 7, m_textures[normal]->GetHandleByRef().GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler2D));
                m_materialEffect->WriteImage(
                    frame, 0, 8, m_textures[arm]->GetHandleByRef().GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler2D));
                m_materialEffect->WriteImage(
                    frame, 0, 9, m_textures[Emissive]->GetHandleByRef().GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler2D));
            }
        },
        updateStruct);
}
std::vector<std::shared_ptr<VTextureAsset>> PBRMaterial::EnumarateTexture()
{
    std::vector<std::shared_ptr<VTextureAsset>> result;
    result.reserve(m_textures.size());
    if(m_materialDescription.features.hasDiffuseTexture)
    {
        result.emplace_back(m_textures[Diffues]);
    }
    if(m_materialDescription.features.hasArmTexture)
    {
        result.emplace_back(m_textures[arm]);
    }
    if(m_materialDescription.features.hasNormalTexture)
    {
        result.emplace_back(m_textures[normal]);
    }
    if(m_materialDescription.features.hasEmissiveTexture)
    {
        result.emplace_back(m_textures[Emissive]);
    }

    return result;
}
std::unordered_map<ETextureType, std::shared_ptr<VTextureAsset>> PBRMaterial::EnumarateTextureMap()
{
    std::unordered_map<ETextureType, std::shared_ptr<VTextureAsset>> result;
    result.reserve(m_textures.size());
    if(m_materialDescription.features.hasDiffuseTexture)
    {
        result[Diffues] = m_textures[Diffues];
    }
    if(m_materialDescription.features.hasArmTexture)
    {
        result[arm] = m_textures[arm];
    }
    if(m_materialDescription.features.hasNormalTexture)
    {
        result[normal] = m_textures[normal];
    }
    if(m_materialDescription.features.hasEmissiveTexture)
    {
        result[Emissive] = m_textures[Emissive];
    }

    return result;
}
}  // namespace ApplicationCore