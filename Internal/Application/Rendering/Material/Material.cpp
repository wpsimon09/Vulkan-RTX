//
// Created by wpsimon09 on 26/11/24.
//

#include "Material.hpp"

#include "Application/AssetsManger/AssetsManager.hpp"
namespace ApplicationCore {
    Material::Material(MaterialPaths& materialPaths,AssetsManager& assets_manager): m_assetManager(assets_manager), m_materialPaths(materialPaths), m_materialDescription()
    {

        if (!materialPaths.DiffuseMapPath.empty())
        {
            m_assetManager.GetTexture(m_textures[MATERIAL_TYPE::PBR_DIFFUSE_MAP], m_materialPaths.DiffuseMapPath);
            m_materialDescription.features.hasDiffuseTexture = true;
        }else
            m_assetManager.GetDummyTexture(m_textures[MATERIAL_TYPE::PBR_DIFFUSE_MAP]);

        if (!materialPaths.NormalMapPath.empty())
        {
            m_assetManager.GetTexture(m_textures[MATERIAL_TYPE::PBR_NORMAL_MAP], m_materialPaths.NormalMapPath);
            m_materialDescription.features.hasNormalTexture = true;
        }else
            m_assetManager.GetDummyTexture(m_textures[MATERIAL_TYPE::PBR_NORMAL_MAP]);

        if (!materialPaths.ArmMapPath.empty())
        {
            m_assetManager.GetTexture(m_textures[MATERIAL_TYPE::PBR_ARM], m_materialPaths.ArmMapPath);
            m_materialDescription.features.hasArmTexture = true;
        }
        else
            m_assetManager.GetDummyTexture(m_textures[MATERIAL_TYPE::PBR_ARM]);
    }

    void Material::Sync()
    {
        m_assetManager.Sync();
    }
} // ApplicationCore