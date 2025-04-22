//
// Created by wpsimon09 on 25/03/25.
//

#ifndef SKYBOXMATERIAL_HPP
#define SKYBOXMATERIAL_HPP
#include "BaseMaterial.hpp"
#include "MaterialStructs.hpp"
#include "Application/Structs/ApplicationStructs.hpp"
#include "Application/Structs/ApplicationStructs.hpp"


namespace ApplicationCore {
class VTextureAsset;
}

namespace ApplicationCore {
class AssetsManager;

class SkyBoxMaterial : public ApplicationCore::BaseMaterial
{
  public:
    SkyBoxMaterial(const std::string& path, AssetsManager& assetsManager);

    void ChangeEffect(std::shared_ptr<VulkanUtils::VRasterEffect> newEffect) override;
    std::shared_ptr<ApplicationCore::VTextureAsset> GetHDRTexture();
    void UpdateGPUTextureData(VulkanUtils::DescriptorSetTemplateVariantRef updateStruct) override;

  private:
    std::shared_ptr<ApplicationCore::VTextureAsset> m_HDRTexture;
    SkyBoxMaterialDescription                       m_skyBoxMaterialDescription;
};

}  // namespace ApplicationCore

#endif  //SKYBOXMATERIAL_HPP
