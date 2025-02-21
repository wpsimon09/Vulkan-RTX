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

namespace VulkanCore
{
    class VImage;
}



inline int MaterialIndexCounter = 0;

namespace ApplicationCore
{
    struct TextureBufferView{
        size_t offset;
        size_t size;
        int materialIndex = -1;
        std::string path;
    }; 

    class AssetsManager;

    class Material
    {
    public:
        explicit Material(MaterialPaths& materialPaths, AssetsManager& assets_manager);

        PBRMaterialDescription& GetMaterialDescription() { return m_materialDescription; };
        std::string& GetMaterialName() { return m_materialName; };
        void SetMaterialname(std::string newName) { m_materialName = std::move(newName); };
        std::shared_ptr<VulkanCore::VImage>& GetTexture(ETextureType type) { return m_textures[type]; }
        bool& IsTransparent() { return m_transparent; }
        void SetTransparent(bool value) { m_transparent = value; }
        TextureBufferView* GetTextureView() { return &m_textureView; }

    private:
        std::string m_materialName;
        std::array<std::shared_ptr<VulkanCore::VImage>,MAX_TEXTURE_COUNT> m_textures;
        PBRMaterialDescription m_materialDescription;
        MaterialPaths m_materialPaths;
        AssetsManager& m_assetManager;
        TextureBufferView m_textureView; // used only for exporting 
        bool m_transparent = false;
        int ID;

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
