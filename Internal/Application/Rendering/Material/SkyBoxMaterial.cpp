//
// Created by wpsimon09 on 25/03/25.
//

#include "SkyBoxMaterial.hpp"

#include "Application/AssetsManger/AssetsManager.hpp"
#include "Application/AssetsManger/EffectsLibrary/EffectsLibrary.hpp"
#include "Application/Logger/Logger.hpp"

namespace ApplicationCore {
    SkyBoxMaterial::SkyBoxMaterial(const std::string& path, AssetsManager& assetsManager)
        :BaseMaterial(assetsManager.GetEffectsLibrary().GetEffect(EEffectType::SkyBox))
    {
       assetsManager.GetHDRTexture(m_HDRTexture, path, false);
    }

    void SkyBoxMaterial::ChangeEffect(std::shared_ptr<VulkanUtils::VEffect> newEffect)
    {
        Utils::Logger::LogError("Can not change effect of SkyBox material");
    }

    std::shared_ptr<ApplicationCore::VTextureAsset> SkyBoxMaterial::GetHDRTexture()
    {
        return m_HDRTexture;;
    }
} // ApplicationCore