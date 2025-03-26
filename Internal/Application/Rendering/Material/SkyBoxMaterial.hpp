//
// Created by wpsimon09 on 25/03/25.
//

#ifndef SKYBOXMATERIAL_HPP
#define SKYBOXMATERIAL_HPP
#include "BaseMaterial.hpp"
#include "MaterialStructs.hpp"


namespace ApplicationCore
{
    class VTextureAsset;
}

namespace ApplicationCore {
    class AssetsManager;

    class SkyBoxMaterial:public ApplicationCore::BaseMaterial {
public:
        SkyBoxMaterial(const std::string& path, AssetsManager& assetsManager);

        void                                                ChangeEffect(std::shared_ptr<VulkanUtils::VEffect> newEffect) override;
        std::shared_ptr<ApplicationCore::VTextureAsset>     GetHDRTexture();
        void UpdateGPUTextureData(VulkanUtils::DescriptorSetTemplateVariant updateStruct) override;

    private:
        std::shared_ptr<ApplicationCore::VTextureAsset> m_HDRTexture;
        SkyBoxMaterialDescription m_skyBoxMaterialDescription;
};

} // ApplicationCore

#endif //SKYBOXMATERIAL_HPP
