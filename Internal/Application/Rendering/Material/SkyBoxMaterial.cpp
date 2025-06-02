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
#include "Application/Structs/ApplicationStructs.hpp"
#include "Application/Structs/ApplicationStructs.hpp"
#include "Vulkan/VulkanCore/Samplers/VSamplers.hpp"
#include "Vulkan/VulkanCore/VImage/VImage2.hpp"

namespace ApplicationCore {
SkyBoxMaterial::SkyBoxMaterial(const std::string& path, AssetsManager& assetsManager)
    : BaseMaterial(assetsManager.GetEffectsLibrary().GetEffect<VulkanUtils::VRasterEffect>(EEffectType::SkyBox))
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

std::vector<std::shared_ptr<VTextureAsset>> SkyBoxMaterial::EnumarateTexture() {
    return {m_HDRTexture};
}
}  // namespace ApplicationCore