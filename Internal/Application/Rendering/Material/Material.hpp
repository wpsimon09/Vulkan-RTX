//
// Created by wpsimon09 on 26/11/24.
//

#ifndef MATERIAL_HPP
#define MATERIAL_HPP
#include <array>
#include <memory>
#include <string>
#include <vector>
#include <glm/vec4.hpp>

#ifndef MAX_TEXTURE_COUNT
 // diffuse, normal, arm ,emissive
 #define MAX_TEXTURE_COUNT 4
#endif

struct PBRMaterialNoTexture
{
    float roughness{0.0f};
    float metalness{0.0f};
    float ao{0.0f};
    float padding{0.0f};

    glm::vec4 diffuse{1.0f, 0.0f, 0.0f, 1.0f};

    glm::vec4 emissive_strength{0.0f,0.0f, 0.0f, 0.0f};
};

struct PBRMaterialFeaturees
{
    bool hasDiffuseTexture{false};
    bool hasEmissiveTexture{false};
    bool hasNormalTexture{false};
    bool hasArmTexture{false};
};

struct MaterialPaths
{
    std::string DiffuseMapPath;
    std::string ArmMapPath;
    std::string NormalMapPath;
};

struct PBRMaterialDescription
{
    PBRMaterialNoTexture values;
    PBRMaterialFeaturees features;
};

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

    void Sync();
private:
    std::array<std::shared_ptr<VulkanCore::VImage>,MAX_TEXTURE_COUNT> m_textures;
    PBRMaterialDescription m_materialDescription;
    MaterialPaths m_materialPaths;
    AssetsManager& m_assetManager;
};

} // ApplicationCore

#endif //MATERIAL_HPP
