
#ifndef VTEXTUREASSET_HPP
#define VTEXTUREASSET_HPP

#include "VAsset.hpp"
#include "Vulkan/VulkanCore/VImage/VImage.hpp"
#include "Vulkan/Global/GlobalVulkanEnums.hpp"

namespace VulkanCore
{
    class VImage2;
}

namespace VulkanUtils
{
    class VTransferOperationsManager;
}

struct TextureBufferInfo;

enum ETextureAssetType{
    Texture = 0,
    EditorBillboard,
};

namespace ApplicationCore {
    class VTextureAsset : public VAsset<VulkanCore::VImage2> {
    public:
        explicit VTextureAsset(const VulkanCore::VDevice& device, std::shared_ptr<VulkanCore::VImage2> defaultTexture, ETextureAssetType type, std::filesystem::path texturePath);
        explicit VTextureAsset(const VulkanCore::VDevice& device, std::shared_ptr<VulkanCore::VImage2> defaultTexture, ETextureAssetType type, TextureBufferInfo& bufferInfo);
        explicit VTextureAsset(const VulkanCore::VDevice& device, std::shared_ptr<VulkanCore::VImage2> texture);

        void Sync() override;
        void Destroy() override;
        void Load() override;


        std::shared_ptr<VulkanCore::VImage2> GetHandle() override;
        VulkanCore::VImage2& GetHandleByRef() override;
    protected:
        void LoadInternal() override;
        void LoadInternalFromBuffer();
    private:

        EImageSource m_textureSource;
        ETextureAssetType m_textureAssetType;
        int m_width;
        int m_height;
        int m_mipLevels;
                
        std::optional<std::filesystem::path> m_originalPathToTexture;
        std::optional<TextureBufferInfo> m_textureBufferInfo;
        std::future<VulkanStructs::ImageData<>> m_loadedImageData;

        VulkanUtils::VTransferOperationsManager& m_transferOpsManager;
    };

}

#endif // VTEXTUREASSET_HPP
