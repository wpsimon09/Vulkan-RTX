//
// Created by wpsimon09 on 26/11/24.
//

#ifndef MATERIAL_HPP
#define MATERIAL_HPP
#include <array>
#include <memory>
#include "Application/Enums/ClientEnums.hpp"


#ifndef MAX_TEXTURE_COUNT
 // diffuse, normal, arm ,emissive
 #define MAX_TEXTURE_COUNT 4
#endif
#include "MaterialStructs.hpp"

namespace VulkanCore
{
    class VImage;
}


namespace ApplicationCore {
    class AssetsManager;

    class Material {

public:
    explicit Material(MaterialPaths& materialPaths, AssetsManager& assets_manager);

    PBRMaterialDescription& GetMaterialDescription(){ return m_materialDescription; };

    std::shared_ptr<VulkanCore::VImage>& GetTexture(MATERIAL_TYPE type){ return m_textures[type];}

private:
    std::array<std::shared_ptr<VulkanCore::VImage>,MAX_TEXTURE_COUNT> m_textures;
    PBRMaterialDescription m_materialDescription;
    MaterialPaths m_materialPaths;
    AssetsManager& m_assetManager;
};

} // ApplicationCore

#endif //MATERIAL_HPP
