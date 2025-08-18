//
// Created by wpsimon09 on 26/11/24.
//

#ifndef PBRMATERIAL_HPP
#define PBRMATERIAL_HPP
#include <array>
#include <memory>
#include <utility>
#include "Application/Enums/ClientEnums.hpp"


#ifndef MAX_TEXTURE_COUNT
// diffuse, normal, arm ,emissive
#define MAX_TEXTURE_COUNT 4
#endif
#include "BaseMaterial.hpp"
#include "MaterialStructs.hpp"
#include "Application/Structs/ApplicationStructs.hpp"
#include "Application/Structs/ApplicationStructs.hpp"

#include <unordered_map>


namespace VulkanUtils {
class VUniformBufferManager;
}

namespace VulkanUtils {
class VRasterEffect;
}

namespace VulkanCore {
class VImage;
}


namespace ApplicationCore {
struct VTextureAsset;
struct TextureBufferView
{
    size_t                          offset;
    size_t                          size;
    int                             widht, height;
    int                             materialIndex = 0;
    std::string                     path;
    ApplicationCore::VTextureAsset* textureAsset;
};

class AssetsManager;

class PBRMaterial : public BaseMaterial
{
  public:
    explicit PBRMaterial(Renderer::EForwardRenderEffects effect, MaterialPaths& materialPaths, AssetsManager& assets_manager);

    PBRMaterialDescription&                          GetMaterialDescription() { return m_materialDescription; }
    std::shared_ptr<ApplicationCore::VTextureAsset>& GetTexture(ETextureType type) { return m_textures[type]; }
    ApplicationCore::VTextureAsset* GetTextureRawPtr(ETextureType type) const { return m_textures[type].get(); }

    MaterialPaths& GetMaterialPaths() { return m_materialPaths; }

    std::vector<std::shared_ptr<VTextureAsset>>                      EnumarateTexture() override;
    std::unordered_map<ETextureType, std::shared_ptr<VTextureAsset>> EnumarateTextureMap();

  private:
    std::array<std::shared_ptr<ApplicationCore::VTextureAsset>, MAX_TEXTURE_COUNT> m_textures;
    PBRMaterialDescription                                                         m_materialDescription;
    MaterialPaths                                                                  m_materialPaths;
};
}  // namespace ApplicationCore

#endif  //MATERIAL_HPP
