//
// Created by wpsimon09 on 25/03/25.
//

#include "SkyBoxMaterial.hpp"

#include "Application/AssetsManger/AssetsManager.hpp"
#include "Application/AssetsManger/EffectsLibrary/EffectsLibrary.hpp"
#include "Application/AssetsManger/Utils/VTextureAsset.hpp"
#include "Application/Logger/Logger.hpp"
#include "Application/Structs/ApplicationStructs.hpp"
#include "Application/Structs/ApplicationStructs.hpp"
#include "Vulkan/VulkanCore/Samplers/VSamplers.hpp"
#include "Vulkan/VulkanCore/VImage/VImage2.hpp"

namespace ApplicationCore {
SkyBoxMaterial::SkyBoxMaterial(const std::string& path, AssetsManager& assetsManager)
    : BaseMaterial(assetsManager.GetEffectsLibrary().GetEffect(EEffectType::SkyBox))
{
    assetsManager.GetHDRTexture(m_HDRTexture, path, true);
}

void SkyBoxMaterial::ChangeEffect(std::shared_ptr<VulkanUtils::VRasterEffect> newEffect)
{
    Utils::Logger::LogError("Can not change effect of SkyBox material");
}

std::shared_ptr<ApplicationCore::VTextureAsset> SkyBoxMaterial::GetHDRTexture()
{
    return m_HDRTexture;
    ;
}

void SkyBoxMaterial::UpdateGPUTextureData(VulkanUtils::DescriptorSetTemplateVariantRef updateStruct)
{
    // sky box can only be one and not the whole variant
    // TODO: sky box will hold env lightning and all that good stuff and pass it to the mateiral here
    //auto& data = std::get<std::reference_wrapper<VulkanUtils::UnlitSingleTexture>>(updateStruct).get();
    //data.texture2D_1 = m_HDRTexture->GetHandle()->GetDescriptorImageInfo(VulkanCore::VSamplers::Sampler2D);
}
}  // namespace ApplicationCore