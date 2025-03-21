//
// Created by wpsimon09 on 26/11/24.
//

#ifndef MATERIAL_HPP
#define MATERIAL_HPP
#include <array>
#include <memory>
#include <utility>
#include "Application/Enums/ClientEnums.hpp"



#ifndef MAX_TEXTURE_COUNT
// diffuse, normal, arm ,emissive
#define MAX_TEXTURE_COUNT 4
#endif
#include "MaterialStructs.hpp"

namespace VulkanUtils
{
    class VEffect;
}

namespace VulkanCore
{
    class VImage;
}


inline int MaterialIndexCounter = 0;

namespace ApplicationCore
{
    struct VTextureAsset;
    struct TextureBufferView{
        size_t offset;
        size_t size;
        int widht, height;
        int materialIndex = -1;
        std::string path;
        ApplicationCore::VTextureAsset* textureAsset;
    }; 

    class AssetsManager;

    class Material
    {
    public:
        explicit Material(MaterialPaths& materialPaths, AssetsManager& assets_manager);
        explicit Material(std::shared_ptr<VulkanUtils::VEffect> materialEffect, MaterialPaths& materialPaths, AssetsManager& assets_manager);

        PBRMaterialDescription&                                 GetMaterialDescription() { return m_materialDescription; }
        std::shared_ptr<VulkanUtils::VEffect>&                  GetEffect();;
        std::shared_ptr<ApplicationCore::VTextureAsset>&        GetTexture(ETextureType type) { return m_textures[type]; }
        std::string&                                            GetMaterialName() { return m_materialName; };
        void                                                    SetMaterialname(std::string newName);
        ApplicationCore::VTextureAsset*                         GetTextureRawPtr(ETextureType type) const { return m_textures[type].get(); }

        MaterialPaths&                                          GetMaterialPaths() { return m_materialPaths; }
        TextureBufferView*                                      GetTextureView() { return &m_textureView; }

        bool&                                                   IsTransparent() { return m_transparent; }
        void                                                    SetTransparent(bool value) { m_transparent = value; }
        void                                                    ChangeEffect(std::shared_ptr<VulkanUtils::VEffect> newEffect);

        bool                                                    IsSavable() const {return m_savable;}
        void                                                    SetSavable(bool savable) {m_savable = savable;}
        void                                                    Update();

    private:
        std::string m_materialName;
        std::array<std::shared_ptr<ApplicationCore::VTextureAsset>,MAX_TEXTURE_COUNT> m_textures;
        PBRMaterialDescription m_materialDescription;
        MaterialPaths m_materialPaths;
        TextureBufferView m_textureView; // used only for exporting
        bool m_transparent = false;
        bool m_savable = false;
        int ID;
        std::shared_ptr<VulkanUtils::VEffect> m_materialEffect;

        friend bool operator==(const Material& lhs, const Material& rhs)
        {
            return lhs.ID == rhs.ID;
        }

        friend bool operator!=(const Material& lhs, const Material& rhs)
        {
            return !(lhs == rhs);
        }
    };
} // ApplicationCore

#endif //MATERIAL_HPP
