//
// Created by wpsimon09 on 26/11/24.
//

#include "PBRMaterial.hpp"
#include "MaterialStructs.hpp"


#include "Application/AssetsManger/AssetsManager.hpp"
#include "Application/AssetsManger/Utils/VTextureAsset.hpp"
#include "Application/Structs/ApplicationStructs.hpp"
#include "Application/Structs/ApplicationStructs.hpp"
#include "Application/Utils/LinearyTransformedCosinesValues.hpp"
#include "Vulkan/Utils/VEffect/VEffect.hpp"
#include "Vulkan/Utils/VUniformBufferManager/VUniformBufferManager.hpp"
#include "Vulkan/VulkanCore/Samplers/VSamplers.hpp"
#include "Vulkan/VulkanCore/VImage/VImage2.hpp"

namespace ApplicationCore {
PBRMaterial::PBRMaterial(std::shared_ptr<VulkanUtils::VEffect> materialEffect, MaterialPaths& materialPaths, AssetsManager& assets_manager)
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

void PBRMaterial::UpdateGPUTextureData(VulkanUtils::DescriptorSetTemplateVariantRef updateStruct)
{
  std::visit(
      [this](auto& descriptorTemplateStruct) {
        auto& effectDstStruct = descriptorTemplateStruct.get();
        using T               = std::decay_t<decltype(effectDstStruct)>;

        if constexpr(std::is_same_v<T, VulkanUtils::BasicDescriptorSet>)
        {
        }
        else if constexpr(std::is_same_v<T, VulkanUtils::UnlitSingleTexture>)
        {
          auto& unlitSingelTextureEffect = static_cast<VulkanUtils::UnlitSingleTexture&>(effectDstStruct);
          unlitSingelTextureEffect.texture2D_1 =
              m_textures[ETextureType::Diffues]->GetHandleByRef().GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler2D);
        }
        else if constexpr(std::is_same_v<T, VulkanUtils::ForwardShadingDstSet>)
        {
          auto& forwardShadingDstSet = static_cast<VulkanUtils::ForwardShadingDstSet&>(effectDstStruct);

          forwardShadingDstSet.texture2D_1 =
              m_textures[Diffues]->GetHandleByRef().GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler2D);

          forwardShadingDstSet.texture2D_2 =
              m_textures[normal]->GetHandleByRef().GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler2D);

          forwardShadingDstSet.texture2D_3 =
              m_textures[arm]->GetHandleByRef().GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler2D);

          forwardShadingDstSet.texture2D_4 =
              m_textures[Emissive]->GetHandleByRef().GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler2D);
        }
      },
      updateStruct);
}
}  // namespace ApplicationCore