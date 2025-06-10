//
// Created by wpsimon09 on 02/12/24.
//

#ifndef MATERIALSTRUCTS_HPP
#define MATERIALSTRUCTS_HPP

#include <string>
#include "glm/glm.hpp"

struct PBRMaterialNoTexture
{

    float roughness{0.4f};
    float metalness{0.2f};
    float ao{0.2f};
    float padding{0.0f};

    glm::vec4 albedo{0.2f, 0.9f, 0.4f, 1.0f};

    glm::vec4 emissive_strength{0.0f, 0.0f, 0.0f, 0.0f};

    friend bool operator==(const PBRMaterialNoTexture& lhs, const PBRMaterialNoTexture& rhs)
    {
        return lhs.roughness == rhs.roughness && lhs.metalness == rhs.metalness && lhs.ao == rhs.ao
               && lhs.padding == rhs.padding && lhs.albedo == rhs.albedo && lhs.emissive_strength == rhs.emissive_strength;
    }

    friend bool operator!=(const PBRMaterialNoTexture& lhs, const PBRMaterialNoTexture& rhs) { return !(lhs == rhs); }
};

struct PBRMaterialFeaturees
{

    int hasAlbedoTexture{false};
    int hasEmissiveTexture{false};
    int hasNormalTexture{false};
    int hasArmTexture{false};


    //===========================================================
    // indexes to the texture array in scene data for ray tracing
    int albedo = -1;
    int armTextureIdx = -1;
    int emissiveTextureIdx = -1;
    int normalTextureIdx = - 1;

    friend bool operator==(const PBRMaterialFeaturees& lhs, const PBRMaterialFeaturees& rhs)
    {
        return lhs.hasAlbedoTexture == rhs.hasAlbedoTexture && lhs.hasEmissiveTexture == rhs.hasEmissiveTexture
               && lhs.hasNormalTexture == rhs.hasNormalTexture && lhs.hasArmTexture == rhs.hasArmTexture;
    }

    friend bool operator!=(const PBRMaterialFeaturees& lhs, const PBRMaterialFeaturees& rhs) { return !(lhs == rhs); }
};

struct MaterialPaths
{
    std::string DiffuseMapPath;
    std::string ArmMapPath;
    std::string NormalMapPath;
    std::string EmmisivePath;
    bool        saveToDisk = false;

    bool operator==(const MaterialPaths& other) const
    {
        return DiffuseMapPath == other.DiffuseMapPath && ArmMapPath == other.ArmMapPath
               && NormalMapPath == other.NormalMapPath && EmmisivePath == other.EmmisivePath;
    }

    bool isTextureLess() const
    {
        return DiffuseMapPath.empty() && ArmMapPath.empty() && NormalMapPath.empty() && EmmisivePath.empty();
    }
};


struct PBRMaterialDescription
{
    friend bool operator==(const PBRMaterialDescription& lhs, const PBRMaterialDescription& rhs)
    {
        return lhs.values == rhs.values && lhs.features == rhs.features;
    }
    friend bool operator!=(const PBRMaterialDescription& lhs, const PBRMaterialDescription& rhs)
    {
        return !(lhs == rhs);
    }
    PBRMaterialNoTexture values;
    PBRMaterialFeaturees features;


};

struct SkyBoxMaterialDescription
{
};

#endif  //MATERIALSTRUCTS_HPP
