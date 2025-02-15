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

    glm::vec4 diffuse{0.2f, 0.9f, 0.4f, 1.0f};

    glm::vec4 emissive_strength{0.0f,0.0f, 0.0f, 0.0f};
};

struct PBRMaterialFeaturees
{
    int hasDiffuseTexture{false};
    int hasEmissiveTexture{false};
    int hasNormalTexture{false};
    int hasArmTexture{false};
};

struct MaterialPaths
{
    std::string DiffuseMapPath;
    std::string ArmMapPath;
    std::string NormalMapPath;

    bool operator==(const MaterialPaths& other) const
    {
        return DiffuseMapPath == other.DiffuseMapPath && ArmMapPath == other.ArmMapPath &&  NormalMapPath == other.NormalMapPath;
    }

    bool isTextureLess() const
    {
        return DiffuseMapPath.empty() && ArmMapPath.empty() && NormalMapPath.empty();
    }
};


struct PBRMaterialDescription
{
    PBRMaterialNoTexture values;
    PBRMaterialFeaturees features;
};

#endif //MATERIALSTRUCTS_HPP
