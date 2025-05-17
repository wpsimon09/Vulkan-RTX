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

void SkyBoxMaterial::UpdateGPUTextureData(EShaderBindingGroup updateStruct, int frame)
{
    // sky box is special case and pushing data of sky box material happens inside scene renderer directly,
    // TODO: this is extremly cluterred and shows that whole effect-material system deserves rewrite
    m_materialEffect->WriteImage(frame, 0, 3, m_HDRTexture->GetHandleByRef().GetDescriptorImageInfo(VulkanCore::VSamplers::SamplerClampToEdge));
}
std::vector<std::shared_ptr<VTextureAsset>> SkyBoxMaterial::EnumarateTexture() {
    return {m_HDRTexture};
}
}  // namespace ApplicationCore