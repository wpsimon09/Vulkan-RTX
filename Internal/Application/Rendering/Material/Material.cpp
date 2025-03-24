//
// Created by wpsimon09 on 26/11/24.
//

#include "Material.hpp"
#include "MaterialStructs.hpp"


#include "Application/AssetsManger/AssetsManager.hpp"
#include "Vulkan/Utils/VEffect/VEffect.hpp"
#include "Vulkan/Utils/VUniformBufferManager/VUniformBufferManager.hpp"

namespace ApplicationCore {
    Material::Material(std::shared_ptr<VulkanUtils::VEffect> materialEffect, MaterialPaths& materialPaths,
        AssetsManager& assets_manager):m_materialEffect(materialEffect), m_textureView(), m_materialPaths(materialPaths)
    {
        ID = ++MaterialIndexCounter;
        m_initialEffect = materialEffect;

        if (!materialPaths.DiffuseMapPath.empty())
        {
            assets_manager.GetTexture(m_textures[ETextureType::Diffues], m_materialPaths.DiffuseMapPath, m_materialPaths.saveToDisk);
            m_materialDescription.features.hasDiffuseTexture = true;
        }else
            assets_manager.GetDummyTexture(m_textures[ETextureType::Diffues]);

        if (!materialPaths.NormalMapPath.empty())
        {
            assets_manager.GetTexture(m_textures[ETextureType::normal], m_materialPaths.NormalMapPath, m_materialPaths.saveToDisk);
            m_materialDescription.features.hasNormalTexture = true;
        }else
            assets_manager.GetDummyTexture(m_textures[ETextureType::normal]);

        if (!materialPaths.ArmMapPath.empty())
        {
            assets_manager.GetTexture(m_textures[ETextureType::arm], m_materialPaths.ArmMapPath, m_materialPaths.saveToDisk);
            m_materialDescription.features.hasArmTexture = true;
        }
        else
            assets_manager.GetDummyTexture(m_textures[ETextureType::arm]);

        //m_assetManager.GetTexture(m_textures[MATERIAL_TYPE::PBR_ARM], m_materialPaths.ArmMapPath);
        m_materialDescription.features.hasEmissiveTexture = false;
        assets_manager.GetDummyTexture(m_textures[ETextureType::emissive]);
    }

    std::shared_ptr<VulkanUtils::VEffect>& Material::GetEffect()
    {
        return m_materialEffect;
    }

    void Material::SetMaterialname(std::string newName)
    {
        m_materialName = newName;
    }

    void Material::ChangeEffect(std::shared_ptr<VulkanUtils::VEffect> newEffect)
    {
        m_materialEffect = newEffect;
    }

    void Material::ResetEffect()
    {
        m_materialEffect = m_initialEffect;
    }
} // ApplicationCore